#include "plain/net/address.h"
#include <cstddef>
#include <iomanip>
#include <cassert>
#include <optional>
#include <ranges>
#include "plain/basic/type/variable.h"
#include "plain/basic/utility.h"
#include "plain/net/detail/socket.h"

using plain::net::Address;

bool plain::net::is_valid_addr(const bytes_t &addr, bool verbose) noexcept {
  constexpr auto key_width {20};
  constexpr auto value_width {10};

  if (verbose) {
    std::cout
      << "Validating socket address: "
      << addr
      << std::endl;
  }

  if (addr.empty()) {
    return false;
  }

  const auto family {detail::get_sa_family(addr)};

  if (verbose) {
    std::cout << std::setw(key_width) << "  Family: "
          << std::right << std::setw(value_width) << family
          << std::endl;
  }

  switch (family) {
    case AF_UNSPEC:
#if !OS_WIN
    case AF_UNIX:
#endif
    case AF_INET:
    case AF_INET6:
      break;
    default:
      return false;
  }

  const auto addr_size {addr.size()};
  const auto addr_size_max {detail::get_sa_size_maximum(family)};
  const auto addr_size_min {detail::get_sa_size_minimum(family)};

  if (verbose) {
    std::cout
      << std::setw(key_width) << "  Actual size: "
      << std::right << std::setw(value_width) << addr_size
      << std::endl
      << std::setw(key_width) << "  Minimum size: "
      << std::right << std::setw(value_width) << addr_size_min
      << std::endl
      << std::setw(key_width) << "  Maximum size: "
      << std::right << std::setw(value_width) << addr_size_max
      << std::endl;
  }

  if (!(addr_size_min <= addr_size && addr_size <= addr_size_max)) {
    return false;
  }

#ifdef HAVE_SOCKADDR_SA_LEN

  const auto sa_len {detail::get_sa_length(addr)};

  if (verbose) {
    std::cout
      << std::setw(key_width) << "  Stored length: "
      << std::right << std::setw(value_width) << sa_len
      << std::endl;
  }

  if (family == AF_UNIX) {
    if (!(addr_size_min <= sa_len && sa_len <= addr_size)) {
      return false;
    }

#if !OS_WIN
    const auto *const sun {get_sun_pointer(addr)};
    const auto sun_len {get_sun_length(sun, addr_size)};

    if (verbose) {
      std::cout
        << std::setw(key_width) << "  Computed length: "
        << std::right << std::setw(value_width) << sun_len
        << std::endl;
    }
#endif

  } else if (family == AF_INET || family == AF_INET6) {
    if (!(sa_len == addr_size)) {
      return false;
    }
  }

#endif

  return true;
}

static bool is_ipv4(std::string_view address) noexcept {
  if (address.find(':') == address.rfind(':')) return true;
  return address.find('.') != std::string::npos;
}

/* std::tuple<ip_v4, ip, port> */
static std::tuple<bool, std::string, uint16_t>
parse_address(std::string_view address) noexcept {
  uint16_t port{0};
  if (address.empty())
    return std::make_tuple(true, "", port);
  using std::operator ""sv;
  if (is_ipv4(address)) {
    size_t pos{std::string::npos};
    if ((pos = address.find(":")) == std::string::npos) {
      return std::make_tuple(true, std::string{address}, port);
    } else {
      std::string ip;
      size_t index{0};
      for (auto str : std::views::split(address, ":"sv)) {
        if (index++ == 0) {
          ip = std::string_view{str};
        } else {
          port = static_cast<uint16_t>(
            plain::toint64(std::string_view{str}.data()));
          break;
        }
      }
      return std::make_tuple(true, ip, port);
    }
  } else { // ip v6
    size_t pos{std::string::npos};
    if ((pos = address.find("]")) == std::string::npos) {
      return std::make_tuple(false, std::string{address}, port);
    } else {
      std::string ip;
      size_t index{0};
      for (auto str : std::views::split(address, "]:"sv)) {
        if (index++ == 0) {
          ip = std::string_view{str};
          if (!ip.empty() && *ip.begin() == '[') ip.erase(ip.begin());
        } else {
          port = static_cast<uint16_t>(
            plain::toint64(std::string_view{str}.data()));
          break;
        }
      }
      return std::make_tuple(false, ip, port);
    }
  }
}

static Address::value_type
to_addr(bool ip_v4, std::string_view ip, uint16_t port, bool listen) {
  Address::value_type r;
  using value_t = Address::value_type::value_type;
  if (ip_v4) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    r.reserve(sizeof addr);
    if (ip.empty() || ip == "127.0.0.1" || ip == "0.0.0.0") {
      if (listen)
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
      else
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    } else {
      addr.sin_addr.s_addr = inet_addr(ip.data());
    }
    if (addr.sin_addr.s_addr == INADDR_NONE)
      throw std::invalid_argument("ip invalid");
    addr.sin_port = htons(port);
    r.append(reinterpret_cast<value_t *>(&addr), sizeof(addr));
  } else {
    sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_flowinfo = 0;
    addr.sin6_scope_id = 0;
    r.reserve(sizeof addr);
    if (ip.empty() || ip == "::" || ip == "0:0:0:0:0:0:0:0" || ip == "::1") {
      if (listen) {
        addr.sin6_addr = in6addr_any;
      } else {
        auto cr = inet_pton(AF_INET6, "::1", &addr.sin6_addr);
        if (cr != 1)
          throw std::invalid_argument("ip invalid");
      }
    } else {
      auto cr = inet_pton(AF_INET6, ip.data(), &addr.sin6_addr);
      if (cr != 1)
        throw std::invalid_argument("ip invalid");
    }
    addr.sin6_port = htons(port);
    r.append(reinterpret_cast<value_t *>(&addr), sizeof(addr));
  }
  return r;
}

static Address::value_type to_addr(std::string_view str, bool listen) {
  auto [ip_v4, ip, port] = parse_address(str);
  return to_addr(ip_v4, ip, port, listen);
}

Address::Address(const value_type &value) : value_{value} {
  if (!is_valid_addr(value_))
    throw std::invalid_argument("Address value is invalid");
}

Address::Address(value_type &&value) : value_{std::move(value)} {
  if (!is_valid_addr(value_))
    throw std::invalid_argument("Address value is invalid");
}
  
Address::Address(std::string_view value, bool listen) :
  value_{to_addr(value, listen)} {
  if (!is_valid_addr(value_))
    throw std::invalid_argument("Address value is invalid");
}

Address::Address(std::string_view ip, uint16_t port, bool listen) :
  value_{to_addr(is_ipv4(ip), ip, port, listen)} {
  if (!is_valid_addr(value_))
    throw std::invalid_argument("Address value is invalid");
}

Address &Address::operator=(const value_type &value) {
  if (!is_valid_addr(value))
    throw std::invalid_argument("Address value is invalid");
  value_ = value;
  return *this;
}

Address::operator bool() noexcept {
  return !value_.empty();
}

Address::value_type Address::data() const noexcept {
  return value_;
  // return detail::get_sa_data(value_);
}

int32_t Address::family() const noexcept {
  return detail::get_sa_family(value_);
}

uint16_t Address::port() const noexcept {
  auto _family = family();
  uint16_t r{0};
  switch (_family) {
    case AF_INET:
      r = detail::get_sin_port(value_);
      break;
    case AF_INET6:
      r = detail::get_sin6_port(value_);
      break;
    default:
      break;
  }
  return r;
}

size_t Address::size() const noexcept {
  return value_.size();
}

std::string Address::text() const noexcept {
  return host() + ":" + std::to_string(port());
}

std::string Address::host() const noexcept {
  auto _family = family();
  std::string r;
  switch (_family) {
    case AF_UNIX:
      r = detail::to_path(value_).value_or("<NULL>");
      break;
    case AF_INET:
      r = detail::to_string(detail::get_sin_addr(value_));
      break;
    case AF_INET6:
      r = detail::to_string(detail::get_sin6_addr(value_));
      break;
    default:
      break;
  }
  return r;
}
