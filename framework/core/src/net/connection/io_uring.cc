#include "plain/net/connection/io_uring.h"
#if __has_include(<liburing.h>)
#include <liburing.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/utsname.h>
#include <sys/poll.h>
// #define PLAIN_LIBURING_ENABLE
#endif
#include "plain/basic/logger.h"

using plain::net::connection::IoUring;
using plain::net::detail::Awaitable;

#ifndef PLAIN_LIBURING_ENABLE

struct IoUring::Impl{};

IoUring::IoUring(const setting_t &setting) : Manager(setting) {

}

IoUring::IoUring(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting) {

}

IoUring::~IoUring() = default;

bool IoUring::prepare() noexcept {
  LOG_ERROR << "disabled";
  return false;
}

bool IoUring::work() noexcept {
  LOG_ERROR << "disabled";
  return false;
}

void IoUring::off() noexcept {

}

bool IoUring::sock_add(
  socket::id_t sock_id, connection::id_t conn_id) noexcept {
  LOG_ERROR << "disabled";
  return false;
}

bool IoUring::sock_remove(socket::id_t sock_id) noexcept {
  LOG_ERROR << "disabled";
  return false;
}

void IoUring::handle_input() noexcept {

}
  
void *IoUring::get_sock_data() noexcept {
  return nullptr;
}

plain::net::detail::Task<int32_t> IoUring::accept_await() noexcept {
  co_await Awaitable{nullptr};
}
#else

struct IoUring::Impl {
  Impl();
  ~Impl();
  socket::id_t ring_fd{socket::kInvalidId};
  socket::id_t epoll_fd{socket::kInvalidId};
  struct io_uring ring;
  uint32_t cqe_count{0};
  bool ready{false};
  static Awaitable await_work(io_uring_sqe *sqe, uint8_t flags);
  [[nodiscard]] struct io_uring_sqe *get_sqe() noexcept;
  bool submit() noexcept;
};

IoUring::Impl::Impl() = default;

IoUring::Impl::~Impl() {
  if (ready) io_uring_queue_exit(&ring);
  if (ring_fd != socket::kInvalidId) close(ring_fd);
  if (epoll_fd != socket::kInvalidId) close(epoll_fd);
}

Awaitable IoUring::Impl::await_work(io_uring_sqe *sqe, uint8_t flags = 0) {
  io_uring_sqe_set_flags(sqe, flags);
  auto set_data = [sqe](void *data) {
    io_uring_sqe_set_data(sqe, data);
  };
  return Awaitable(set_data);
}

struct io_uring_sqe *IoUring::Impl::get_sqe() noexcept {
  auto r = io_uring_get_sqe(&ring);
  if (!!r) [[likely]]
    return r;

  if (submit())
    r = io_uring_get_sqe(&ring);
  return r;
}

bool IoUring::Impl::submit() noexcept {
  io_uring_cq_advance(&ring, cqe_count);
  cqe_count = 0;
  auto error = io_uring_submit(&ring);
  if (error < 0) {
    LOG_ERROR << "error: " << -error;
    return false;
  }
  return true;
}

IoUring::IoUring(const setting_t &setting) :
  Manager(setting), impl_{std::make_unique<Impl>()} {

}

IoUring::IoUring(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting),
  impl_{std::make_unique<Impl>()} {
}

IoUring::~IoUring() = default;

bool IoUring::prepare() noexcept {
  impl_->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (impl_->epoll_fd < 0) {
    LOG_ERROR << "epoll_create1 error: " << impl_->epoll_fd;
    return false;
  }
  impl_->ring_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (impl_->ring_fd < 0) {
    LOG_ERROR << "eventfd error: " << impl_->ring_fd;
    return false;
  }

  int32_t error{0};
  if ((error = io_uring_queue_init(setting_.max_count, &impl_->ring, 0)) < 0) {
    LOG_ERROR << "io_uring_queue_init error: " << error;
    return false;
  }

  impl_->ready = true;
  return true;
}

bool IoUring::work() noexcept {
  for (;;) {
    accept_await(); // accept.
                    // recv ?
    io_uring_submit_and_wait(&impl_->ring, 1);
    io_uring_cqe *cqe{nullptr};
    uint32_t head{0};
    io_uring_for_each_cqe(&impl_->ring, head, cqe) {
      ++impl_->cqe_count;
      auto coro =
        static_cast<net::detail::Resolver *>(io_uring_cqe_get_data(cqe));
      if (coro) coro->resolve(cqe->res);
    }
    if (!running()) break;
  }
  return true;
}

void IoUring::off() noexcept {

}

bool IoUring::sock_add(
  socket::id_t sock_id, connection::id_t conn_id) noexcept {
  auto sqe = impl_->get_sqe();
  io_uring_prep_poll_add(sqe, sock_id, POLLIN);
  return true;
}

bool IoUring::sock_remove(socket::id_t sock_id) noexcept {
  auto sqe = impl_->get_sqe();
  io_uring_prep_poll_remove(sqe, sock_id);
  return true;
}

void IoUring::handle_input() noexcept {

}

void *IoUring::get_sock_data() noexcept {
  return impl_->get_sqe();
}

plain::net::detail::Task<int32_t>
IoUring::accept_await() noexcept {
  auto sqe = impl_->get_sqe();
  io_uring_prep_accept(sqe, listen_fd_, nullptr, nullptr, 0);
  auto fd = co_await impl_->await_work(sqe);
  if (fd != socket::kInvalidId) {
    accept(fd);
  }
}

#endif
