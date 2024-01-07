#include "plain/net/connection/io_uring.h"
#if __has_include(<liburing.h>)
#include <liburing.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/utsname.h>
#endif
#include "plain/basic/logger.h"
#include "plain/net/connection/detail/coroutine.h"

using plain::net::connection::IoUring;

#if !__has_include(<liburing.h>)
IoUring::IoUring(const setting_t &setting) : Manager(setting) {

}
  
IoUring::IoUring(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting) {

}

IoUring::~IoUring() = default;

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

plain::net::connection::detail::Task IoUring::work_recurrence() noexcept {
  return {};
}
  
void IoUring::handle_input() noexcept {

}

#else

struct IoUring::Impl {
  socket::id_t ring_fd;
  socket::id_t epoll_fd;
  struct io_uring ring;
};

IoUring::IoUring(const setting_t &setting) :
  Manager(setting), impl_{std::make_shared<Impl>()} {

}
  
IoUring::IoUring(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting),
  impl_{std::make_shared<Impl>()} {

}

IoUring::~IoUring() = default;

bool IoUring::work() noexcept {

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

  return true;
}
  
void IoUring::off() noexcept {

}
  
bool IoUring::sock_add(
  socket::id_t sock_id, connection::id_t conn_id) noexcept {

  return true;
}
  
bool IoUring::sock_remove(socket::id_t sock_id) noexcept {

  return true;
}

plain::net::connection::detail::Task IoUring::work_recurrence() noexcept {
  return {};
}
  
void IoUring::handle_input() noexcept {

}

#endif
