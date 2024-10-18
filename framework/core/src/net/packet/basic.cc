#include "plain/net/packet/basic.h"

using plain::net::packet::Basic;

enum {
  kReadableFlag = 0,
  kWriteableFlag = 1,
  kCallRequestFlag = 2,
  kCallResponseFlag = 3,
};

struct Basic::Impl {
  id_t id;
  bytes_t data;
  size_t offset{0};
  uint8_t flag{0};
  bool have_flag(uint32_t flag) const noexcept;
  void set_flag(bool flag, uint32_t type) noexcept;
};

bool Basic::Impl::have_flag(uint32_t type) const noexcept {
  return flag & (1 << type);
}

void Basic::Impl::set_flag(bool _flag, uint32_t type) noexcept {
  if (_flag)
    flag |= (1 << type);
  else
    flag &= ~(1 << kReadableFlag);
}

size_t Basic::write(std::string_view str) {
  if (!impl_->have_flag(kWriteableFlag)) return 0;
  if (str.size() == 0) return 0;
  const auto *buffer = reinterpret_cast<const std::byte *>(str.data());
  auto size = str.size();
  *this << static_cast<uint32_t>(size);
  impl_->data.append(buffer, size);
  return size;
}

size_t Basic::write(const bytes_t &bytes) {
  if (!impl_->have_flag(kWriteableFlag)) return 0;
  *this << static_cast<uint32_t>(bytes.size());
  impl_->data.append(bytes);
  return bytes.size();
}

size_t Basic::write(const std::byte *bytes, size_t length) {
  if (!impl_->have_flag(kWriteableFlag)) return 0;
  impl_->data.append(bytes, length);
  return length;
}

size_t Basic::write(const_byte_span_t bytes) {
  if (!impl_->have_flag(kWriteableFlag)) return 0;
  impl_->data.append(bytes.data(), bytes.size());
  return bytes.size();
}

size_t Basic::read(std::string &str) {
  if (!impl_->have_flag(kReadableFlag)) return 0;
  uint32_t length{0};
  *this >> length;
  if (length == 0) return 0;
  auto left = impl_->data.size() - impl_->offset;
  if (left < length) return 0;
  const auto *buffer = reinterpret_cast<const char *>(
    impl_->data.data() + impl_->offset);
  str.append(buffer, length);
  impl_->offset += length;
  return str.size();
}

size_t Basic::read(bytes_t &bytes) {
  if (!impl_->have_flag(kReadableFlag)) return 0;
  uint32_t length{0};
  *this >> length;
  if (length == 0) return 0;
  auto left = impl_->data.size() - impl_->offset;
  if (left < length) return 0;
  const auto *buffer = impl_->data.data() + impl_->offset;
  bytes.insert(0, buffer, length);
  impl_->offset += length;
  return bytes.size();
}

size_t Basic::read(std::byte *value, size_t length) {
  if (!value || !impl_->have_flag(kReadableFlag)) return 0;
  auto left = impl_->data.size() - impl_->offset;
  if (left < length) return 0;
  const auto *buffer = impl_->data.data() + impl_->offset;
  std::memcpy(value, buffer, length);
  impl_->offset += length;
  return length;
}

size_t Basic::remove(size_t length) noexcept {
  auto left = impl_->data.size() - impl_->offset;
  if (left < length) {
    impl_->offset += left;
    return left;
  }
  impl_->offset += length;
  return length;
}
  
plain::const_byte_span_t Basic::data() const noexcept {
  return as_const_bytes(impl_->data);
}
  
size_t Basic::offset() const noexcept {
  return impl_->offset;
}

void Basic::set_readable(bool flag) noexcept {
  impl_->set_flag(flag, kReadableFlag);
}
  
void Basic::set_writeable(bool flag) noexcept {
  impl_->set_flag(flag, kWriteableFlag);
}

void Basic::set_call_request(bool flag) noexcept {
  impl_->set_flag(flag, kCallRequestFlag);
}
  
void Basic::set_call_response(bool flag) noexcept {
  impl_->set_flag(flag, kCallResponseFlag);
}
  
bool Basic::is_call_request() const noexcept {
  return impl_->have_flag(kCallRequestFlag);
}
  
bool Basic::is_call_response() const noexcept {
  return impl_->have_flag(kCallResponseFlag);
}

Basic::Basic() : impl_{std::make_unique<Impl>()} {

}

Basic::~Basic() = default;
  
void Basic::set_id(id_t id) noexcept {
  impl_->id = id;
}

plain::net::packet::id_t Basic::id() const noexcept {
  return impl_->id;
}
