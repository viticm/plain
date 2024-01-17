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
#include "plain/net/packet/config.h"
#include "plain/net/socket/config.h"
#include "plain/net/stream/codec.h"

namespace plain::net {

class PLAIN_API Connector {

 public:
  Connector(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting = {});
  Connector(const setting_t &setting = {});
  ~Connector();

 public:
  bool start();
  void stop();

 public:
  std::shared_ptr<connection::Basic>
  connect(
    std::string_view address,
    const std::chrono::milliseconds 
    &timeout = std::chrono::seconds(5)) noexcept;
  std::shared_ptr<connection::Basic>
  connect(
    std::string_view ip, uint16_t port,
    const std::chrono::milliseconds
    &timeout = std::chrono::seconds(5)) noexcept;

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  const stream::codec_t &codec() const noexcept;
  void set_packet_dispatcher(packet::dispatch_func func) noexcept;
  const packet::dispatch_func &dispatcher() const noexcept;
  std::shared_ptr<connection::Basic> get_conn(id_t id) const noexcept;
  bool is_full() const noexcept;
  void broadcast(std::shared_ptr<packet::Basic> packet) noexcept;
  concurrency::executor::Basic &get_executor();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:
  std::shared_ptr<connection::Basic>
  connect_impl(
    std::string_view addr_or_ip, uint16_t port = 0,
    const std::chrono::milliseconds &timeout = {}) noexcept;

};

} // namespace plain::net

#endif // PLAIN_NET_CONNECTOR_H_
