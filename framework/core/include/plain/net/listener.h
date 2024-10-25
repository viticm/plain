/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id listener.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/07 21:20
 * @uses The listener for net implemention.
 */

#ifndef PLAIN_NET_LISTENER_H_
#define PLAIN_NET_LISTENER_H_

#include "plain/net/config.h"
#include "plain/concurrency/config.h"
#include "plain/net/connection/config.h"
#include "plain/net/packet/config.h"
#include "plain/net/socket/config.h"
#include "plain/net/stream/codec.h"
#include "plain/net/rpc/dispatcher.h"

namespace plain::net {

class PLAIN_API Listener final {

 public:
  Listener(
    const setting_t &setting = {},
    std::shared_ptr<concurrency::executor::Basic> executor = {});

  ~Listener();

 public:
  bool start();
  void stop();

 public:
  template <typename F> void bind(const std::string &name, F func) {
    rpc_dispatcher_->bind(name, func);
  }

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  const stream::codec_t &codec() const noexcept;
  void set_dispatcher(packet::dispatch_func func) noexcept;
  const packet::dispatch_func &dispatcher() const noexcept;
  void set_connect_callback(connection::callable_func func) noexcept;
  void set_disconnect_callback(connection::callable_func func) noexcept;
 
 public:
  std::shared_ptr<connection::Basic>
  get_conn(connection::id_t id) const noexcept;
  bool is_full() const noexcept;
  void broadcast(std::shared_ptr<packet::Basic> packet) noexcept;
  std::shared_ptr<concurrency::executor::Basic> get_executor() const noexcept;
  bool running() const noexcept;

 public:
  uint64_t send_size() const noexcept;
  uint64_t recv_size() const noexcept;
 
 public:
  Address address() const noexcept;

 private:
  std::shared_ptr<rpc::Dispatcher> rpc_dispatcher_;
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace plain::net

#endif // PLAIN_NET_LISTENER_H_
