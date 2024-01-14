/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/06 10:01
 * @uses The net socket basic implemention.
 */

#ifndef PLAIN_NET_SOCKET_BASIC_H_
#define PLAIN_NET_SOCKET_BASIC_H_

#include "plain/net/socket/config.h"
#include "plain/net/address.h"

namespace plain::net {
namespace socket {

class PLAIN_API Basic : noncopyable {

 public:
  Basic(id_t id = kInvalidSocket);
  virtual ~Basic();

 public:
  Basic(Basic &&object) noexcept;

 public:
  bool create();
  bool create(int32_t domain, int32_t type, int32_t protocol = 0);
  bool close() noexcept;
  id_t release() noexcept;
  Basic clone() noexcept;
  bool valid() const noexcept;
  explicit operator bool() const noexcept {
    return valid();
  }
  bool error() const noexcept;
  int32_t send(const bytes_t &bytes, uint32_t flag = 0);
  int32_t recv(bytes_t &bytes, uint32_t flag = 0); // recv max bytes capacity size.
  size_t avail() const noexcept;

 public:
  Address address() const;
  Address peer_address() const;
  bool get_option(int32_t level, int32_t name, void *val, uint32_t *len);
  template <typename T>
  bool get_option(int32_t level, int32_t name, T *val) {
    uint32_t len = sizeof(T);
    return get_option(level, name, reinterpret_cast<void *>(val), &len);
  }
  id_t id() const noexcept;
  bool set_recv_size(uint32_t size) const;
  uint32_t get_recv_size() const;
  bool set_send_size(uint32_t size) const;
  uint32_t get_send_size() const;
  bool set_id(id_t id) noexcept;

 public:
  bool shutdown(int32_t how = 0x2) noexcept; // default: SHUT_RDWR = 2
  bool set_nonblocking(bool on = true) noexcept;
  bool is_nonblocking() const noexcept;
  bool bind(const Address &addr);
  bool bind();
  uint32_t get_linger() const noexcept;
  bool set_linger(uint32_t lingertime) noexcept;
  bool set_reuse_addr(bool on = true) const noexcept;
  bool listen(uint32_t backlog);
  int32_t accept(Address &addr);
  int32_t accept();
  bool connect(
    std::string_view address,
    const std::chrono::milliseconds &timeout = {}) noexcept;
  bool connect(
    std::string_view ip, uint16_t port,
    const std::chrono::milliseconds &timeout = {}) noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace socket
} // namespace plain::net

#endif // PLAIN_NET_SOCKET_BASIC_H_
