/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id manager.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/22 20:20
 * @uses The net connection manager class implemention.
 */

#ifndef PLAIN_NET_CONNECTION_MANAGER_H_
#define PLAIN_NET_CONNECTION_MANAGER_H_

#include "plain/net/connection/config.h"
#include "plain/concurrency/config.h"
#include "plain/net/packet/config.h"
#include "plain/net/socket/config.h"
#include "plain/net/stream/codec.h"

namespace plain::net {
namespace connection {

class PLAIN_API Manager : 
  noncopyable, public std::enable_shared_from_this<Manager> {

 public:
  Manager(const setting_t &setting); // Default use thread executor.
  Manager(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting);
  virtual ~Manager();

 public:
  bool start();
  void stop();

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  const stream::codec_t &codec() const noexcept;
  void set_packet_dispatcher(packet::dispatch_func func) noexcept;
  const packet::dispatch_func &dispatcher() const noexcept;
  std::shared_ptr<Basic> get_conn(id_t id) const noexcept;
  bool is_full() const noexcept;
  void broadcast(std::shared_ptr<packet::Basic> packet) noexcept;
  concurrency::executor::Basic &get_executor();
 
 protected:
  virtual bool work() noexcept = 0; // working
  virtual void off() noexcept = 0; // off work
  virtual bool sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept = 0;
  virtual bool sock_remove(socket::id_t sock_id) noexcept = 0;
  std::shared_ptr<Basic> accept() noexcept;

 protected:
  std::shared_ptr<Basic> new_conn() noexcept;
  void remove(std::shared_ptr<Basic> conn) noexcept;
  void remove(connection::id_t conn_id) noexcept;
  void foreach(std::function<void(std::shared_ptr<Basic> conn)> func); // valid

 protected:
  setting_t setting_;
  socket::id_t listen_fd_{socket::kInvalidSocket};
  std::shared_ptr<socket::Listener> listen_sock_;

 private:
  friend class Basic;
  friend class net::Connector;
  friend class net::Listener;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_MANAGER_H_
