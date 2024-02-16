/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id io_uring.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/02/06 10:26
 * @uses The io uring socket implemention.
 */

#ifndef PLAIN_NET_SOCKET_IO_URING_H_
#define PLAIN_NET_SOCKET_IO_URING_H_

#include "plain/net/socket/config.h"
#include "plain/net/socket/basic.h"

namespace plain::net {
namespace socket {

class PLAIN_API alignas(kCacheInlineAlignment)
IoUring final : public Basic {

 public:
  IoUring();
  virtual ~IoUring();

 public:
  constexpr bool awaitable() const override {
    return true;
  }
  detail::Awaitable send_await(
    const bytes_t &bytes, uint32_t flag, void *udata) override;
  detail::Awaitable recv_await(
    bytes_t &bytes, uint32_t flag, void *udata) override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

}; 

} // namespace socket
} // namespace plain::net

#endif // PLAIN_NET_SOCKET_IO_URING_H_
