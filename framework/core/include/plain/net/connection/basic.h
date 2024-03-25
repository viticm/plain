/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/22 18:08
 * @uses The net connection basic class implemention.
 */

#ifndef PLAIN_NET_CONNECTION_BASIC_H_
#define PLAIN_NET_CONNECTION_BASIC_H_

#include "plain/net/connection/config.h"
#include "plain/net/packet/config.h"
#include "plain/net/stream/codec.h"

namespace plain::net {
namespace connection {

class PLAIN_API alignas(kCacheInlineAlignment)
Basic final : noncopyable {

 public:
  Basic();
  Basic(std::shared_ptr<socket::Basic> socket);
  Basic(Basic &&object) noexcept;
  ~Basic();

 public:
  Basic &operator=(Basic &&object) noexcept;

 public:
  bool init() noexcept;
  bool idle() const noexcept;
  bool shutdown(int32_t how = 0x2) noexcept; // default: SHUT_RDWR = 2
  bool close() noexcept;
  bool valid() const noexcept;

 public:
  id_t id() const noexcept;
  std::string name() const noexcept;
  std::shared_ptr<socket::Basic> socket() const noexcept;
  void set_name(std::string_view name) noexcept;

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  void set_connect_callback(callable_func func) noexcept;
  void set_disconnect_callback(callable_func func) noexcept;
  void set_dispatcher(packet::dispatch_func func) noexcept;

 public:
  bool send(const std::shared_ptr<packet::Basic> &packet) noexcept;

 private:
  friend class Manager;
  friend class Epoll;
  friend class Select;
  friend class Iocp;
  friend class IoUring;
  friend class Kqueue;
  friend class net::Connector;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:
  void on_connect() noexcept;
  void on_disconnect() noexcept;

 private:
  void set_id(id_t id) noexcept;
  void set_manager(std::shared_ptr<Manager> manager) noexcept;
  void set_keep_alive(bool flag) const noexcept;
  bool is_keep_alive() const noexcept;

 private:
  bool work() noexcept;
  void enqueue_work(WorkFlag flag) noexcept;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_BASIC_H_
