#include "plain/net/stream/basic.h"
#include <cassert>
#include "plain/basic/ring.h"
#include "plain/net/socket/basic.h"

using plain::net::stream::Basic;
using plain::net::socket::kErrorWouldBlock;
using plain::net::socket::kSocketError;

struct Basic::Impl {
  std::weak_ptr<socket::Basic> weak_socket;
  DynamicRing<std::byte, 128> buffer;
  std::string encrypt_key;
  bool compressed{false};
};

Basic::Basic(std::shared_ptr<socket::Basic> socket) :
  impl_{std::make_unique<Impl>()} {
  impl_->weak_socket = socket;
}

Basic::~Basic() = default;

int32_t Basic::pull() noexcept {
  auto socket = impl_->weak_socket.lock();
  if (!socket || !socket->valid()) return 0;
  auto socket_avail = socket->avail();
  if (socket_avail == 0) return 0;
  auto once_max = impl_->buffer.write_avail() + impl_->buffer.size();
  bytes_t bytes;
  bytes.reserve(socket_avail >= once_max ? once_max : socket_avail);
  auto e = socket->recv(bytes);
  if (e == kErrorWouldBlock) return 0;
  if (e == kSocketError) return kSocketError - 1;
  if (e == 0) return kSocketError - 2;
  uint32_t size = e;
  auto read_size = impl_->buffer.write(bytes.data(), size);
  if (read_size < size) return kSocketError - 3;
  return read_size;
}

int32_t Basic::push() noexcept {
  // std::cout << "push" << this << std::endl;
  auto socket = impl_->weak_socket.lock();
  if (!socket || !socket->valid()) return 0;
  auto send_size = impl_->buffer.read_avail();
  // std::cout << "need size: " << send_size << std::endl;
  if (send_size == 0) return 0;
  constexpr auto once_max = 1024;
  if (send_size > once_max)
    send_size = once_max;
  bytes_t bytes;
  bytes.resize(send_size);
  send_size = impl_->buffer.read(bytes.data(), send_size, true); // Read to temp.
  // std::cout << "push value: " << (char *)bytes.data() << std::endl;
  decltype(send_size) real_send_size{0};
  for (uint16_t i = 0; i < 99; ++i) {
    if (real_send_size >= send_size) break;
    bytes_t temp;
    temp.insert(0, bytes.data() + real_send_size, send_size - real_send_size);
    auto send_result = socket->send(temp);
    if (send_result == kErrorWouldBlock || send_result == 0) break;
    if (send_result == kSocketError) return kSocketError - 1;
    assert(send_result > 0);
    impl_->buffer.remove(send_result); // Sended then remove of buffer.
    real_send_size += send_result;
  }
  return real_send_size;
}

bool Basic::full() const noexcept {
  return impl_->buffer.full();
}

bool Basic::empty() const noexcept {
  return impl_->buffer.empty();
}
  
size_t Basic::size() const noexcept {
  return impl_->buffer.read_avail();
}
  
void Basic::clear() noexcept {
  impl_->buffer.consumer_clear();
}
  
std::shared_ptr<plain::net::socket::Basic> Basic::socket() {
  return impl_->weak_socket.lock();
}

bool Basic::encrypted() const noexcept {
  return !impl_->encrypt_key.empty();
}
  
void Basic::set_encrypt(std::string_view key) noexcept {
  impl_->encrypt_key = key;
}// empty key is close encrypt
  
bool Basic::compressed() const noexcept {
  return impl_->compressed;
}
  
void Basic::set_compress(bool on) noexcept {
  impl_->compressed = on;
}

size_t Basic::write(std::string_view str) {
  const auto *buffer = reinterpret_cast<const std::byte *>(str.data());
  return impl_->buffer.write(buffer, str.size());
}

size_t Basic::write(const bytes_t &bytes) {
  return impl_->buffer.write(bytes.data(), bytes.size());
}

size_t Basic::write(const_byte_span_t bytes) {
  return impl_->buffer.write(bytes.data(), bytes.size());
}

size_t Basic::read(std::string &str) {
  uint32_t length{0};
  *this >> length;
  if (length == 0) return 0;
  str.reserve(length);
  auto *buffer = reinterpret_cast<std::byte *>(str.data());
  return impl_->buffer.read(buffer, length);
}

size_t Basic::read(bytes_t &bytes) {
  uint32_t length{0};
  *this >> length;
  if (length == 0) return 0;
  bytes.reserve(length);
  return impl_->buffer.read(bytes.data(), length);
}

size_t Basic::read(std::byte *value, size_t length) {
  if (!value) return 0;
  return impl_->buffer.read(value, length);
}

size_t Basic::remove(size_t length) noexcept {
  return impl_->buffer.remove(length);
}
  
size_t Basic::peek(std::byte *value, size_t length) {
  if (!value) return 0;
  return impl_->buffer.read(value, length, true);
}
