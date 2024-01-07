/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id listener.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/17 14:09
 * @uses The net socket listener implemention.
 */

#ifndef PLAIN_NET_SOCKET_LISTENER_H_
#define PLAIN_NET_SOCKET_LISTENER_H_

#include "plain/net/socket/config.h"
#include "plain/net/address.h"

namespace plain::net {
namespace socket {

class PLAIN_API Listener : noncopyable {

 public:
  Listener();
  ~Listener() noexcept;

 public:
  bool init(const Address &addr, uint32_t backlog = 5);

 public:
  void close();
  bool accept(std::shared_ptr<Basic> socket);
 
 public:
  uint32_t get_linger() const noexcept;
  bool set_linger(uint32_t lingertime) noexcept;
  bool is_nonblocking() const noexcept;
  bool set_nonblocking(bool on = true) noexcept;
  uint32_t get_recv_size() const noexcept;
  void set_recv_size(uint32_t size) noexcept;
  uint32_t get_send_size() const noexcept;
  void set_send_size(uint32_t size) noexcept;
  id_t id() const noexcept;
  Address address() const noexcept;

 private:
  std::unique_ptr<Basic> socket_;

};

} // namespace socket
} // namespace plain::net

#endif // PLAIN_NET_SOCKET_LISTENER_H_
