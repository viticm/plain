/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id io_uring.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/04 17:58
 * @uses The io uring for connection implemention.
 */

#ifndef PLAIN_NET_CONNECTION_IO_URING_H_
#define PLAIN_NET_CONNECTION_IO_URING_H_

#include "plain/net/connection/config.h"
#include "plain/net/detail/coroutine.h"
#include "plain/net/connection/manager.h"
#include "plain/net/connection/detail/config.h"

namespace plain::net {
namespace connection {

class PLAIN_API alignas(kCacheInlineAlignment)
IoUring final : public Manager {

 public:
  IoUring(const setting_t &setting);
  IoUring(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting);
  virtual ~IoUring();

 protected:
  bool prepare() noexcept override;
  bool work() noexcept override;
  void off() noexcept override;
  bool sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept override;
  bool sock_remove(socket::id_t sock_id) noexcept override;
  void *get_sock_data() noexcept override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:
  void handle_input() noexcept;
  net::detail::Task<int32_t> accept_await() noexcept;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_IO_URING_H_
