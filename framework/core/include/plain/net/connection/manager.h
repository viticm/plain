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
#include "plain/engine/config.h"
#include "plain/net/packet/config.h"
#include "plain/net/socket/config.h"
#include "plain/net/stream/codec.h"
#include "plain/net/rpc/config.h"

namespace plain::net {
namespace connection {

class PLAIN_API Manager : 
  noncopyable, public std::enable_shared_from_this<Manager> {

 public:
  // Default executor is worker thread(one thread).
  Manager(
    const setting_t &setting,
    std::shared_ptr<concurrency::executor::Basic> executor);
  virtual ~Manager();

 public:
  bool start();
  void stop();
  bool running() const noexcept;

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  void set_dispatcher(packet::dispatch_func func) noexcept;
  void set_connect_callback(callable_func func) noexcept;
  void set_disconnect_callback(callable_func func) noexcept;

 public:
  std::shared_ptr<Basic> get_conn(id_t id) const noexcept;
  bool is_full() const noexcept;
  void broadcast(std::shared_ptr<packet::Basic> packet) noexcept;
  std::shared_ptr<concurrency::executor::Basic> get_executor() const noexcept;
  bool send_ctrl_cmd(std::string_view cmd) noexcept;
  void execute(std::function<void()> func); // Multi safe execute.
  size_t size() const noexcept;

 public:
  uint64_t send_size() const noexcept;
  uint64_t recv_size() const noexcept;
 
 protected:
  virtual bool work() noexcept = 0; // working
  virtual void off() noexcept = 0; // off work
  virtual bool sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept = 0;
  virtual bool sock_remove(socket::id_t sock_id) noexcept = 0;
  std::shared_ptr<Basic> accept() noexcept;
  std::shared_ptr<Basic> accept(socket::id_t sock_id) noexcept;
  virtual bool prepare() noexcept { return true; }
  virtual void *get_sock_data() noexcept { // send/recv need data.
    return nullptr;
  }

 protected:
  std::shared_ptr<Basic> new_conn() noexcept;
  void remove(
    std::shared_ptr<Basic> conn, bool no_event = false,
    bool sock = true) noexcept;
  void remove(
    connection::id_t conn_id, bool no_event = false, bool sock = true) noexcept;
  void foreach(std::function<void(std::shared_ptr<Basic> conn)> func); // valid
  void recv_ctrl_cmd() noexcept;
  void enqueue(connection::id_t conn_id) noexcept;
  void set_name(connection::id_t conn_id, std::string_view name) noexcept;
  std::string get_name(connection::id_t conn_id) const noexcept;
  rpc::Dispatcher *rpc_dispatcher() const noexcept;
  void set_rpc_dispatcher(std::shared_ptr<rpc::Dispatcher> dispatcher) noexcept;

 protected:
  void increase_send_size(size_t size);
  void increase_recv_size(size_t size);

 protected:
  const stream::codec_t &codec() const noexcept;
  const packet::dispatch_func &dispatcher() const noexcept;
  const callable_func &connect_callback() const noexcept;

 protected:
  setting_t setting_;
  socket::id_t listen_fd_{socket::kInvalidId};
  std::shared_ptr<socket::Listener> listen_sock_;
  socket::id_t ctrl_read_fd_{socket::kInvalidId};

 private:
  friend class Basic;
  friend class net::Connector;
  friend class net::Listener;
  friend class plain::Kernel;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_MANAGER_H_
