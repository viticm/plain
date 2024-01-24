/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id kevent.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/18 10:01
 * @uses The connection manager with kqueue implemention.
 */

#ifndef PLAIN_NET_CONNECTION_KQUEUE_H_
#define PLAIN_NET_CONNECTION_KQUEUE_H_

#include "plain/net/connection/config.h"
#include "plain/net/connection/manager.h"

namespace plain::net {
namespace connection {

class PLAIN_API alignas(kCacheInlineAlignment)
Kqueue final : public Manager {

 public:
  Kqueue(const setting_t &setting);
  Kqueue(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting);
  virtual ~Kqueue();

 protected:
  bool prepare() noexcept override;
  bool work() noexcept override;
  void off() noexcept override;
  bool sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept override;
  bool sock_remove(socket::id_t sock_id) noexcept override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:
  void handle_input() noexcept;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_KQUEUE_H_
