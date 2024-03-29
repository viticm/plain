#include "plain/net/connection/epoll.h"
#if __has_include(<sys/epoll.h>)
#include <sys/epoll.h>
#include <poll.h>
#include <signal.h>
#define PLAIN_EPOLL_ENABLE
#endif
#include <cassert>
#include "plain/basic/logger.h"
#include "plain/basic/utility.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/file/api.h"
#include "plain/net/connection/basic.h"
#include "plain/net/socket/basic.h"

namespace plain::net::connection {

namespace {

#ifdef PLAIN_EPOLL_ENABLE

static constexpr size_t kOnceAcccpetCount{64};

struct data_struct {
  int32_t fd{socket::kInvalidId};
  int32_t max_count{std::numeric_limits<int32_t>::max()};
  std::atomic_int32_t result_event_count{0};
  int32_t event_index{0};
  struct epoll_event *events{nullptr};
};
using data_t = data_struct;

int32_t poll_create(data_t &d, int32_t max_count) {
  int32_t fd = epoll_create(max_count);
  if (fd != -1) {
    d.fd = fd;
    d.max_count = max_count;
    d.events = new epoll_event[max_count];
    assert(d.events);
    // signal(SIGPIPE, SIG_IGN);
  } else {
    perror("poll_create failed");
  }
  return fd;
}

int32_t poll_add(data_t &d, int32_t fd, int32_t mask, id_t conn_id) {
  struct epoll_event _epoll_event;
  memset(&_epoll_event, 0, sizeof(_epoll_event));
  _epoll_event.events = mask;
  _epoll_event.data.u64 =
    touint64(static_cast<uint32_t>(fd), static_cast<uint32_t>(conn_id));
  int32_t r = epoll_ctl(d.fd, EPOLL_CTL_ADD, fd, &_epoll_event);
  return r;
}

/*
int32_t poll_mod(data_t &d, int32_t fd, int32_t mask) {
  struct epoll_event _epoll_event;
  memset(&_epoll_event, 0, sizeof(_epoll_event));
  _epoll_event.events = mask;
  _epoll_event.data.fd = fd;
  int32_t r = epoll_ctl(d.fd, EPOLL_CTL_MOD, fd, &_epoll_event);
  return r;
}
*/

int32_t poll_delete(data_t &d, int32_t fd) {
  struct epoll_event _epoll_event;
  memset(&_epoll_event, 0, sizeof(_epoll_event));
  _epoll_event.events = EPOLLIN | EPOLLET;
  _epoll_event.data.fd = fd;
  int32_t r = epoll_ctl(d.fd, EPOLL_CTL_DEL, fd, &_epoll_event);
  return r;
}

int32_t poll_wait(data_t &d, int32_t timeout) {
  assert(d.events);
  assert(d.max_count > 0);
  d.result_event_count = epoll_wait(d.fd, d.events, d.max_count, timeout);
  d.event_index = 0;
  return d.result_event_count;
}

int32_t poll_destory(data_t &d) {
  if (d.fd == -1) return 0;
  plain::close(d.fd);
  safe_delete_array(d.events);
  d.fd = -1;
  d.events = nullptr;
  return 0;
}

/*
int32_t poll_event(data_t &d, int32_t *fd, int32_t *events) {
  int32_t r{0};
  if (d.event_index < d.result_event_count) {
    struct epoll_event &_epoll_event = d.events[d.event_index++];
    *events = _epoll_event.events;
    *fd = _epoll_event.data.fd;
  } else {
    r = -1;
  }
  return r;
}
*/

#endif

}

struct Epoll::Impl {
#ifdef PLAIN_EPOLL_ENABLE
  data_t data;
  std::mutex mutex;
#endif
};

Epoll::Epoll(
  const setting_t &setting,
  std::shared_ptr<concurrency::executor::Basic> executor) :
  Manager(setting, executor), impl_{std::make_unique<Impl>()} {
}
  
Epoll::~Epoll() {
#ifdef PLAIN_EPOLL_ENABLE
  poll_destory(impl_->data);
#endif
}
  
bool Epoll::prepare() noexcept {
#ifdef PLAIN_EPOLL_ENABLE
  if (running()) return true;
  auto fd = poll_create(impl_->data, setting_.max_count);
  if (fd <= 0) {
    LOG_ERROR << setting_.name << " create error max_count: "
      << setting_.max_count << " fd: " << fd;
    return false;
  }
  if (listen_fd_ != socket::kInvalidId) {
    auto r = poll_add(impl_->data, listen_fd_, EPOLLIN, connection::kInvalidId);
    if (r < 0) {
      LOG_ERROR << setting_.name << " add error result: " << r;
      return false;
    }
  }
  return true;
#else
  return false;
#endif
}

bool Epoll::work() noexcept {
#ifdef PLAIN_EPOLL_ENABLE
  poll_wait(impl_->data, -1);
  if (impl_->data.result_event_count < 0) {
    LOG_ERROR << "error: " << impl_->data.result_event_count;
    return false;
  }
  handle_input();
  return true;
#else
  return false;
#endif
}

void Epoll::off() noexcept {
#ifdef PLAIN_EPOLL_ENABLE
  // poll_destory(impl_->data);
#endif
}

bool Epoll::sock_add(
  [[maybe_unused]] socket::id_t sock_id,
  [[maybe_unused]] connection::id_t conn_id) noexcept {
  assert(sock_id != socket::kInvalidId);
  assert(conn_id != connection::kInvalidId);
#ifdef PLAIN_EPOLL_ENABLE
  if (poll_add(impl_->data, sock_id, EPOLLIN | EPOLLET, conn_id) != 0) {
    LOG_ERROR << setting_.name << " sock_add error: " << strerror(errno);
  } else {
    return true;
  }
#endif
  return false;
}
  
bool Epoll::sock_remove([[maybe_unused]] socket::id_t sock_id) noexcept {
  assert(sock_id >= 0);
  assert(sock_id != socket::kInvalidId);
#ifdef PLAIN_EPOLL_ENABLE
  if (poll_delete(impl_->data, sock_id) != 0) {
    LOG_ERROR << setting_.name << " sock_remove error: " << strerror(errno);
  } else {
    return true;
  }
#endif
  return false;
}

void Epoll::handle_input() noexcept {
#ifdef PLAIN_EPOLL_ENABLE
  if (!running()) return;
  size_t accept_count{0};
  auto &d = impl_->data;
  for (int32_t i = 0; i < d.result_event_count; ++i) {
    if (!running()) break;
    auto sock_id = static_cast<socket::id_t>(
      get_highsection(d.events[i].data.u64));
    auto conn_id = static_cast<connection::id_t>(
      get_lowsection(d.events[i].data.u64));
    if (sock_id != socket::kInvalidId &&
        sock_id == listen_fd_ && accept_count < kOnceAcccpetCount) {
      ++accept_count;
      this->accept();
    } else if (sock_id != socket::kInvalidId && sock_id == ctrl_read_fd_) {
      recv_ctrl_cmd();
    } else if (d.events[i].events & EPOLLIN) {
      if (conn_id == connection::kInvalidId)
        continue;
      auto conn = get_conn(conn_id);
      if (!conn) {
        LOG_ERROR << setting_.name << " can't find connection: " << conn_id;
        continue;
      }
      if (sock_id == socket::kInvalidId) {
        LOG_ERROR << setting_.name << " can't find socket: " << conn_id;
        remove(conn_id);
        continue;
      }
      if (conn->socket()->error()) {
        LOG_ERROR << setting_.name << " socket error: " << conn_id;
        remove(conn_id);
        continue;
      }
      conn->enqueue_work(WorkFlag::Input);
    }
  }
#endif
}

} // namespace plain::net::connection
