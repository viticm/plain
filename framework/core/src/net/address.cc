#include "plain/net/address.h"
#include <cstddef>
#include <iomanip>
#include <cassert>
#include <optional>
#include "plain/basic/type/variable.h"
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

Address::Address(const value_type &value) : value_{value} {
  if (!is_valid_addr(value_))
    throw std::invalid_argument("Address value is invalid");
}

Address::Address(value_type &&value) : value_{std::move(value)} {
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
