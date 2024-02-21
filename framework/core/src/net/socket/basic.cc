#include "plain/net/socket/basic.h"
#include "plain/basic/logger.h"
#include "plain/net/socket/api.h"
#include "plain/net/detail/socket.h"

using plain::net::socket::Basic;

struct Basic::Impl {
  id_t id{kInvalidId};
};

Basic::Basic(id_t id) : impl_{std::make_unique<Impl>()} {
  impl_->id = id;
}

Basic::~Basic() {
  if (static_cast<bool>(impl_)) close();
}

Basic::Basic(Basic &&object) noexcept = default;

bool Basic::create() {
  if (!close()) return false;
  impl_->id = socket::create(AF_INET, SOCK_STREAM, 0); // default is ip_v4
  return valid();
}

bool Basic::create(int32_t domain, int32_t type, int32_t protocol) {
  if (!close()) return false;
  impl_->id = socket::create(domain, type, protocol);
  return valid();
}

bool Basic::close() noexcept {
  if (impl_->id == kInvalidId) return true;
  return socket::close(release());
}

plain::net::socket::id_t Basic::release() noexcept {
  auto r = impl_->id;
  impl_->id = kInvalidId;
  return r;
}

Basic Basic::clone() noexcept {
  if (impl_->id == kInvalidId) return {};
  id_t id{kInvalidId};
#if OS_WIN
  WSAPROTOCOL_INFOW prot_info;
  if (::WSADuplicateSocketW(impl_->id, ::GetCurrentProcessId(), &prot_info) == 0)
    id = ::WSASocketW(
      AF_INET, SOCK_STREAM, 0, &prot_info, 0, WSA_FLAG_OVERLAPPED);
#elif OS_UNIX
  id = dup(impl_->id);
#endif
  return Basic{id};
}

bool Basic::valid() const noexcept {
  return impl_->id != kInvalidId && !error();
}

bool Basic::error() const noexcept {
  if (impl_->id == kInvalidId) return true;
  int32_t value{0};
  uint32_t length{static_cast<uint32_t>(sizeof(value))};
  getsockoptb(impl_->id, SOL_SOCKET, SO_ERROR, &value, &length);
  return value != 0;
}

int32_t Basic::send(const bytes_t &bytes, uint32_t flag) {
  if (!valid()) return 0;
  auto size = static_cast<uint32_t>(bytes.size());
  return socket::send(impl_->id, bytes.data(), size, flag);
}
  
int32_t Basic::recv(bytes_t &bytes, uint32_t flag) {
  if (!valid()) return 0;
  auto size = static_cast<uint32_t>(bytes.capacity());
  return socket::recv(impl_->id, bytes.data(), size, flag);
}
  
size_t Basic::avail() const noexcept {
  if (!valid()) return 0;
  return socket::available(impl_->id);
}

plain::net::Address Basic::address() const {
  auto store = sockaddr_storage{};
  int32_t len = sizeof(store);
  auto e = getsockname(impl_->id, reinterpret_cast<sockaddr *>(&store), &len);
  if (e == kInvalidId)
    return Address{};
  Address::value_type value;
  value.reserve(len);
  value.insert(
    0, reinterpret_cast<Address::value_type::value_type *>(&store), len);
  return Address{value};
}

plain::net::Address Basic::peer_address() const {
  auto store = sockaddr_storage{};
  int32_t len = sizeof(store);
  auto e = getpeername(impl_->id, reinterpret_cast<sockaddr *>(&store), &len);
  if (e == kInvalidId)
    return Address{};
  Address::value_type value;
  value.reserve(len);
  value.insert(
    0, reinterpret_cast<Address::value_type::value_type *>(&store), len);
  return Address{value};
}

bool Basic::get_option(int32_t level, int32_t name, void *val, uint32_t *len) {
  return getsockoptb(impl_->id, level, name, val, len);
}

plain::net::socket::id_t Basic::id() const noexcept {
  return impl_->id;
}

bool Basic::set_id(id_t id) noexcept {
  if (!close()) return false;
  impl_->id = id;
  return true;
}

bool Basic::set_recv_size(uint32_t size) const {
  auto r = setsockopt(impl_->id, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
  return r;
}

uint32_t Basic::get_recv_size() const {
  uint32_t value{0};
  uint32_t length{sizeof(value)};
  getsockoptb(impl_->id, SOL_SOCKET, SO_RCVBUF, &value, &length);
  return value;
}

bool Basic::set_send_size(uint32_t size) const {
  auto r = setsockopt(impl_->id, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
  return r;
}

uint32_t Basic::get_send_size() const {
  uint32_t value{0};
  uint32_t length{sizeof(value)};
  getsockoptb(impl_->id, SOL_SOCKET, SO_SNDBUF, &value, &length);
  return value;
}

bool Basic::shutdown(int32_t how) noexcept {
  if (!valid()) return true;
  return socket::shutdown(impl_->id, how);
}
  
bool Basic::set_nonblocking(bool on) noexcept {
  if (!valid()) return false;
  return socket::set_nonblocking(impl_->id, on);
}
  
bool Basic::is_nonblocking() const noexcept {
  if (!valid()) return false;
  return socket::get_nonblocking(impl_->id);
}
  
bool Basic::bind(const Address &addr) {
  if (!valid()) return false;
  auto d = addr.data();
  auto ptr = reinterpret_cast<sockaddr *>(d.data());
  auto size = static_cast<uint32_t>(d.size());
  return socket::bind(impl_->id, ptr, size);
}
  
bool Basic::bind() {
  if (!valid()) return false;
  return socket::bind(impl_->id, nullptr, 0);
}
  
uint32_t Basic::get_linger() const noexcept {
  if (!valid()) return 0;
  uint32_t result{0};
  struct linger getlinger;
  uint32_t length = sizeof(getlinger);
  getsockoptb(impl_->id, SOL_SOCKET, SO_LINGER, &getlinger, &length);
  result = getlinger.l_linger;
  return result;
}
  
bool Basic::set_linger(uint32_t lingertime) noexcept {
  struct linger setlinger;
  setlinger.l_onoff = lingertime > 0 ? 1 : 0;
  setlinger.l_linger = static_cast<uint16_t>(lingertime);
  auto r = socket::setsockopt(
    impl_->id, SOL_SOCKET, SO_LINGER, &setlinger, sizeof(setlinger));
  return r;
}
  
bool Basic::set_reuse_addr(bool on) const noexcept {
  bool r{true};
  int32_t option = true == on ? 1 : 0;
  r = setsockopt(impl_->id, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  return r;
}
  
bool Basic::listen(uint32_t backlog) {
  if (!valid()) return false;
  return socket::listen(impl_->id, backlog);
}
  
int32_t Basic::accept(Address &addr) {
  int32_t r{kInvalidId};
  if (!valid()) return r;
  auto d = addr.data();
  uint32_t len{static_cast<uint32_t>(d.size())};
  auto ptr = reinterpret_cast<sockaddr *>(d.data());
  r = socket::accept(impl_->id, ptr, &len);
  return r;
}
  
int32_t Basic::accept() {
  int32_t r{kInvalidId};
  if (!valid()) return r;
  r = socket::accept(impl_->id, nullptr, 0);
  return r;
}

bool Basic::connect(
  std::string_view address, const std::chrono::milliseconds &timeout) noexcept {
  bool r{false};
  try {
    Address addr{address, false};
    if (!valid() && (!close() || !create(addr.family(), SOCK_STREAM, 0)))
      return false;
    const auto data{addr.data()};

    auto ptr = detail::get_sa_pointer(data); // can't use data.data(), memory cut
    auto len = static_cast<uint32_t>(data.size());
    r = socket::connect(impl_->id, ptr, len, timeout);
  } catch(...) {
    LOG_ERROR << "connect except";
  }
  return r;
}

bool Basic::connect(
  std::string_view ip, uint16_t port,
  const std::chrono::milliseconds &timeout) noexcept {
  bool r{false};
  try {
    Address addr{ip, port, false};
    if (!valid() && (!close() || !create(addr.family(), SOCK_STREAM, 0)))
      return false;
    const auto data{addr.data()};
    auto ptr = detail::get_sa_pointer(data); // can't use data.data(), memory cut
    auto len = static_cast<uint32_t>(data.size());
    r = socket::connect(impl_->id, ptr, len, timeout);
  } catch(...) {
    LOG_ERROR << "connect except";
  }
  return r;
}
