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

namespace plain::net {

class PLAIN_API Listener final {

 public:
  Listener(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting = {});
  Listener(const setting_t &setting = {});

  ~Listener();

 public:
  bool start();
  void stop();

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  const stream::codec_t &codec() const noexcept;
  void set_dispatcher(packet::dispatch_func func) noexcept;
  const packet::dispatch_func &dispatcher() const noexcept;
  void set_connect_callback(connection::callable_func func) noexcept;
  void set_disconnect_callback(connection::callable_func func) noexcept;
 
 public:
  std::shared_ptr<connection::Basic> get_conn(id_t id) const noexcept;
  bool is_full() const noexcept;
  void broadcast(std::shared_ptr<packet::Basic> packet) noexcept;
  concurrency::executor::Basic &get_executor();
 
 public:
  Address address() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace plain::net

#endif // PLAIN_NET_LISTENER_H_
