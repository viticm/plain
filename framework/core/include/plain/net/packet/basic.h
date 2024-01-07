/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/20 18:22
 * @uses The net packet basic class implemention.
 */

#ifndef PLAIN_NET_PACKET_BASIC_H_
#define PLAIN_NET_PACKET_BASIC_H_

#include "plain/net/packet/config.h"
#include "plain/basic/type/byte.h"
#include "plain/basic/endian.h"

namespace plain::net {
namespace packet {

class PLAIN_API Basic {

 public:
  Basic();
  ~Basic();

 public:
  size_t write(std::string_view str);
  size_t write(const bytes_t &bytes);
  size_t read(std::string &str);
  size_t read(bytes_t &bytes);
  size_t read(std::byte *value, size_t length);
  size_t remove(size_t length) noexcept;
  const_byte_span_t data() const noexcept;

 public:
  void set_readable(bool flag) noexcept;
  void set_writeable(bool flag) noexcept;

 public:
  void set_id(id_t id) noexcept;
  id_t id() const noexcept;

 public:
  template <typename T>
  Basic &operator<<(const T &value) {
    bytes_t bytes;
    bytes.reserve(sizeof(T));
    auto temp = hton(value);
    bytes.insert(0, reinterpret_cast<bytes_t::value_type *>(&temp), sizeof(T));
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
    read(reinterpret_cast<std::byte *>(value), sizeof(T));
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

bool dispatch(std::shared_ptr<Basic> packet) noexcept;

} // namespace packet
} // namespace plain::net

#endif // PLAIN_NET_PACKET_BASIC_H_
