/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/17 17:43
 * @uses The net stream basic class implemention.
 */

#ifndef PLAIN_NET_STREAM_BASIC_H_
#define PLAIN_NET_STREAM_BASIC_H_

#include "plain/net/stream/config.h"
#include "plain/basic/type/byte.h"
#include "plain/basic/endian.h"

namespace plain::net {
namespace stream {

class PLAIN_API Basic : noncopyable {

 public:
  Basic(std::shared_ptr<socket::Basic> socket);
  virtual ~Basic();

 public:
  int32_t pull() noexcept; // socket -> buffer
  int32_t push() noexcept; // buffer -> socket

 public:
  bool full() const noexcept;
  bool empty() const noexcept;
  size_t size() const noexcept;
  void clear() noexcept;
  std::shared_ptr<socket::Basic> socket();

 public:
  bool encrypted() const noexcept;
  void set_encrypt(std::string_view key) noexcept; // empty key is close encrypt
  bool compressed() const noexcept;
  void set_compress(bool on = true) noexcept;
  
 public:
  size_t write(std::string_view str);
  size_t write(const bytes_t &bytes);
  size_t write(const_byte_span_t bytes);
  size_t read(std::string &str);
  size_t read(bytes_t &bytes);
  size_t read(std::byte *value, size_t length);
  size_t remove(size_t length) noexcept;
  size_t peek(std::byte *value, size_t length);

 public:
  template <typename T>
  Basic &operator<<(const T &value) {
    auto temp = hton(value);
    auto bytes = as_const_bytes(&temp, sizeof(temp));
    write(bytes);
    return *this;
  }

  Basic &operator<<(std::string_view str) {
    write(str);
    return *this;
  }

  Basic &operator<<(const bytes_t &bytes) {
    write(bytes);
    return *this;
  }

  template <typename T>
  Basic &operator>>(T &value) {
    value = T{};
    read(reinterpret_cast<std::byte *>(&value), sizeof(T));
    value = ntoh(value);
    return *this;
  }

  Basic &operator>>(std::string &str) {
    read(str);
    return *this;
  }

  Basic &operator>>(bytes_t &bytes) {
    read(bytes);
    return *this;
  }

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace stream
} // namespace plain::net

#endif // PLAIN_NET_STREAM_BASIC_H_
