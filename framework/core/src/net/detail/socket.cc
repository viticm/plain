#include "plain/net/detail/socket.h"
#include <cassert>

#if !OS_WIN
constexpr auto sun_size {sizeof(sockaddr_un)};
constexpr auto sun_path_offset {offsetof(sockaddr_un, sun_path)};
static constexpr auto sun_len_max {sun_size};
static constexpr auto sun_len_min {sun_path_offset};
#endif

constexpr auto sa_family_offset {offsetof(sockaddr, sa_family)};
constexpr auto sin6_family_offset {offsetof(sockaddr_in6, sin6_family)};
constexpr auto sa_data_offset {offsetof(sockaddr, sa_data)};
constexpr auto ss_size {sizeof(sockaddr_storage)};
constexpr auto sin_port_offset {offsetof(sockaddr_in, sin_port)};
constexpr auto sin6_port_offset {offsetof(sockaddr_in6, sin6_port)};
constexpr auto sin_size {sizeof(sockaddr_in)};
constexpr auto sin6_size {sizeof(sockaddr_in6)};
constexpr auto sin_addr_offset {offsetof(sockaddr_in, sin_addr)};
constexpr auto sin6_addr_offset {offsetof(sockaddr_in6, sin6_addr)};

namespace plain::net::detail {

std::string to_string(const in_addr &addr) noexcept {
  std::string r{INET_ADDRSTRLEN, '\0'};
  ::inet_ntop(AF_INET, &addr, r.data(), r.size());
  return r;
}

std::string to_string(const in6_addr &addr) noexcept {
  std::string r{INET6_ADDRSTRLEN, '\0'};
  ::inet_ntop(AF_INET6, &addr, r.data(), r.size());
  return r;
}

const sockaddr *get_sa_pointer(const bytes_t &addr) noexcept {
  const void *p = addr.data();
  return static_cast<const sockaddr *>(p);
}

sockaddr *get_sa_pointer(bytes_t &addr) noexcept {
  void *p = addr.data();
  return static_cast<sockaddr *>(p);
}

const sockaddr_in *get_sin_pointer(const bytes_t &addr) noexcept {
  const void *p = addr.data();
  return static_cast<const sockaddr_in *>(p);
}

sockaddr_in *get_sin_pointer(bytes_t &addr) noexcept {
  void *p = addr.data();
  return static_cast<sockaddr_in *>(p);
}

const sockaddr_in6 *get_sin6_pointer(const bytes_t &addr) noexcept {
  const void *p = addr.data();
  return static_cast<const sockaddr_in6 *>(p);
}

sockaddr_in6 *get_sin6_pointer(bytes_t &addr) noexcept {
  void *p = addr.data();
  return static_cast<sockaddr_in6 *>(p);
}

#if !OS_WIN
const sockaddr_un *get_sun_pointer(const bytes_t &addr) noexcept {
  const void *pointer = addr.data();
  return static_cast<const sockaddr_un *>(pointer);
}

sockaddr_un *get_sun_pointer(bytes_t &addr) noexcept {
  void *pointer = addr.data();
  return static_cast<sockaddr_un *>(pointer);
}

const char *get_path_pointer(const sockaddr_un *sun) noexcept {
  return static_cast<const char *>(sun->sun_path);
}

char *get_path_pointer(sockaddr_un *sun) noexcept {
  return static_cast<char *>(sun->sun_path);
}

const char *get_path_pointer(const bytes_t &addr) noexcept {
  const auto *p = get_sun_pointer(addr);
  return get_path_pointer(p);
}

char *get_path_pointer(bytes_t &addr) noexcept {
  auto *p = get_sun_pointer(addr);
  return get_path_pointer(p);
}
#endif

int32_t get_sa_family(const bytes_t &addr) noexcept {
  if (addr.empty()) return 0;
  const auto *const sa {get_sa_pointer(addr)};
  if (addr.size() < sa_family_offset + sizeof sa->sa_family)
    return 0;
  return sa->sa_family;
}

size_t get_sa_size_minimum(int32_t family) noexcept {
  switch (family) {
    case AF_UNSPEC:
      return ss_size;
#if !OS_WIN
    case AF_UNIX:
      return sun_len_min;
#endif
    case AF_INET:
      return sin_size;
    case AF_INET6:
      return sin6_size;
    default:
      break;
  }
  return 0;
}

size_t get_sa_size_maximum(int32_t family) noexcept {
  switch (family) {
    case AF_UNSPEC:
      return ss_size;
#if !OS_WIN
    case AF_UNIX:
      return sun_len_max;
#endif
    case AF_INET:
      return sin_size;
    case AF_INET6:
      return sin6_size;
    default:
      break;
  }
  return 0;
}

uint16_t get_sin_port(const bytes_t &addr) noexcept {
  assert(get_sa_family(addr) == AF_INET);
  const auto *const sin {get_sin_pointer(addr)};
  if (addr.size() < sin_port_offset + sizeof sin->sin_port)
    return 0;
  return ntohs(sin->sin_port);
}

uint16_t get_sin6_port(const bytes_t &addr) noexcept {
  assert(get_sa_family(addr) == AF_INET6);
  const auto *const sin6 {get_sin6_pointer(addr)};
  if (addr.size() < sin6_port_offset + sizeof sin6->sin6_port)
    return 0;
  return ntohs(sin6->sin6_port);
}

in_addr 
get_sin_addr(const bytes_t &addr, const in_addr &sin_addr) noexcept {
  assert(get_sa_family(addr) == AF_INET);
  const auto *const sin {get_sin_pointer(addr)};
  if (addr.size() < sin_addr_offset + sizeof sin->sin_addr)
    return sin_addr;
  return sin->sin_addr;
}

in6_addr
get_sin6_addr(const bytes_t &addr, const in6_addr &sin6_addr) noexcept {
  assert(get_sa_family(addr) == AF_INET6);
  const auto *const sin6 {get_sin6_pointer(addr)};
  if (addr.size() < sin6_addr_offset + sizeof sin6->sin6_addr)
    return sin6_addr;
  return sin6->sin6_addr;
}

std::optional<std::string>
to_path([[maybe_unused]] const bytes_t &addr) noexcept {
#if !OS_WIN
  if (get_sa_family(addr) != AF_UNIX || addr.size() <= sun_path_offset)
    return {};
  const auto *const data {get_path_pointer(addr)};
  auto size_max {addr.size() - sun_len_min};
  auto size {::strnlen(data, size_max)};
  return std::string {data, size};
#else
  return {};
#endif
}

} // plain::net::detail
