/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id address.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/10/16 18:07
 * @uses The net address implemention.
 */

#ifndef PLAIN_NET_ADDRESS_H_
#define PLAIN_NET_ADDRESS_H_

#include "plain/net/config.h"
#include "plain/basic/type/config.h"

namespace plain::net {

class PLAIN_API Address {

 public:
  using value_type = bytes_t;

 public:
  Address() noexcept = default;
  explicit Address(const value_type &value);
  explicit Address(value_type &&value);
  Address(std::string_view value, bool listen = true);
  Address(std::string_view ip, uint16_t port, bool listen = true);
  Address(const Address &value) = default;
  Address(Address &&value) = default;
  ~Address() noexcept = default;

 public:
  Address &operator=(const Address &value) noexcept = default;
  Address &operator=(Address &&value) noexcept = default;
  Address &operator=(const value_type &value);
  explicit operator bool() noexcept;

 public:
  [[nodiscard]] value_type data() const noexcept;
  [[nodiscard]] int32_t family() const noexcept;
  [[nodiscard]] uint16_t port() const noexcept;
  [[nodiscard]] size_t size() const noexcept;
  [[nodiscard]] std::string text() const noexcept;
  [[nodiscard]] std::string host() const noexcept;

 private:
  value_type value_;

};

bool is_valid_addr(const bytes_t &addr, bool verbose = false) noexcept;

} // namespace plain::net

#endif // PLAIN_NET_ADDRESS_H_
