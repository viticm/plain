#include "plain/net/socket/io_uring.h"
#if __has_include(<liburing.h>)
#include <liburing.h>
// #define PLAIN_LIBURING_ENABLE
#endif

/**
 * If want zero copy memory, need use the io_uring_prep_write/read interfaces.
 **/

using plain::net::socket::IoUring;
using plain::net::detail::Awaitable;

struct IoUring::Impl {
#ifdef PLAIN_LIBURING_ENABLE
  static Awaitable await_work(io_uring_sqe *sqe, uint8_t flags);
#endif
};

#ifdef PLAIN_LIBURING_ENABLE
Awaitable IoUring::Impl::await_work(io_uring_sqe *sqe, uint8_t flags = 0) {
  io_uring_sqe_set_flags(sqe, flags);
  auto set_data = [sqe](void *data) {
    io_uring_sqe_set_data(sqe, data);
  };
  return Awaitable(set_data);
}

#endif

IoUring::IoUring() = default;
IoUring::~IoUring() = default;

Awaitable
IoUring::send_await(const bytes_t &bytes, uint32_t flag, void *udata) {
#ifdef PLAIN_LIBURING_ENABLE
  auto sqe = static_cast<io_uring_sqe *>(udata);
  io_uring_prep_send(sqe, this->id(), bytes.data(), bytes.size(), 0);
  return impl_->await_work(sqe);
#else
  return Awaitable{nullptr};
#endif
}

Awaitable IoUring::recv_await(bytes_t &bytes, uint32_t flag, void *udata) {
#ifdef PLAIN_LIBURING_ENABLE
  auto sqe = static_cast<io_uring_sqe *>(udata);
  io_uring_prep_recv(sqe, this->id(), bytes.data(), bytes.size(), 0);
  return impl_->await_work(sqe);
#else
  return Awaitable{nullptr};
#endif
}
