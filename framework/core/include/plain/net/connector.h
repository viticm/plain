/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id connector.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/07 21:20
 * @uses The connector for net implemention.
 */

#ifndef PLAIN_NET_CONNECTOR_H_
#define PLAIN_NET_CONNECTOR_H_

#include "plain/net/config.h"
#include "plain/concurrency/config.h"
#include "plain/net/connection/config.h"
#include "plain/net/packet/config.h"
#include "plain/net/socket/config.h"
#include "plain/net/stream/codec.h"

namespace plain::net {

class PLAIN_API Connector final {

 public:
  Connector(
    const setting_t &setting = {},
    std::shared_ptr<concurrency::executor::Basic> executor = {});
  ~Connector();

 public:
  bool start();
  void stop();

 public:
  std::shared_ptr<connection::Basic>
  connect(
    std::string_view address,
    std::function<bool(connection::Basic *)> init_func = {},
    const std::chrono::milliseconds 
    &timeout = std::chrono::seconds(5),
    socket::Type sock_type = socket::Type::Tcp) noexcept;
  std::shared_ptr<connection::Basic>
  connect(
    std::string_view ip, uint16_t port,
    std::function<bool(connection::Basic *)> init_func = {},
    const std::chrono::milliseconds
    &timeout = std::chrono::seconds(5),
    socket::Type sock_type = socket::Type::Tcp) noexcept;

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

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:
  std::shared_ptr<connection::Basic>
  connect_impl(
    std::string_view addr_or_ip, uint16_t port = 0,
    std::function<bool(connection::Basic *)> init_func = {},
    const std::chrono::milliseconds &timeout = {},
    socket::Type sock_type = socket::Type::Tcp) noexcept;

};

} // namespace plain::net

#endif // PLAIN_NET_CONNECTOR_H_
