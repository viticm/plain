/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id epoll.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/12/19 17:19
 * @uses The plain net connection epoll implemention.
 */

#ifndef PLAIN_NET_CONNECTION_EPOLL_H_
#define PLAIN_NET_CONNECTION_EPOLL_H_

#include "plain/net/connection/config.h"
#include "plain/net/connection/manager.h"
#include "plain/net/connection/detail/config.h"

namespace plain::net {
namespace connection {

class PLAIN_API Epoll : public Manager {

 public:
  Epoll(const setting_t &setting);
  Epoll(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting);
  virtual ~Epoll();

 protected:
  bool work() noexcept override;
  void off() noexcept override;
  bool sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept override;
  bool sock_remove(socket::id_t sock_id) noexcept override;

 private:
  struct Impl;
  std::shared_ptr<Impl> impl_;

 private:
  detail::Task work_recurrence() noexcept;
  void handle_input() noexcept;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_EPOLL_H_
