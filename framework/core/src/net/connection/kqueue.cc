#include "plain/net/connection/kqueue.h"
#if __has_include(<sys/event.h>)
#include <sys/event.h>
#include <netdb.h>
#include <sys/types.h>
#define ENABLE_KQUEUE
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

#ifdef ENABLE_KQUEUE

static constexpr size_t kOnceAcccpetCount{64};

struct data_struct {
  int32_t fd{socket::kInvalidId};
  int32_t max_count{std::numeric_limits<int32_t>::max()};
  std::atomic_int32_t result_event_count{0};
  int32_t event_index{0};
  struct kevent *events{nullptr};
  std::vector<uint64_t> event_datas; // For events save conn and sock ids.
  uint64_t listen_data{0};
  uint64_t read_data{0};
};
using data_t = data_struct;

int32_t poll_create(data_t &d, int32_t max_count) {
  int32_t fd = kqueue();
  if (fd != -1) {
    d.fd = fd;
    d.max_count = max_count;
    d.events = new struct kevent[max_count];
    assert(d.events);
    d.event_datas.resize(max_count);
    // signal(SIGPIPE, SIG_IGN);
  } else {
    perror("poll_create failed");
  }
  return fd;
}

int32_t poll_delete(data_t &d, int32_t fd) {
  struct kevent event;
  EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  kevent(d.fd, &event, 1, nullptr, 0, nullptr);
  EV_SET(&event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
  kevent(d.fd, &event, 1, nullptr, 0, nullptr);
  return 0;
}

int32_t poll_add(data_t &d, int32_t fd, id_t conn_id) {
  if (conn_id != connection::kInvalidId &&
      (conn_id > d.event_datas.size() || d.fd == socket::kInvalidId)) {
    return -1;
  }
  struct kevent event;
  std::memset(&event, 0, sizeof event);
  auto ud = conn_id == connection::kInvalidId ? &d.listen_data : &d.read_data;
  if (conn_id > 0) {
    size_t index = conn_id - 1;
    ud = &d.event_datas[index];
  }
  *ud = touint64(static_cast<uint32_t>(fd), static_cast<uint32_t>(conn_id));
  EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, ud);
  if (kevent(d.fd, &event, 1, nullptr, 0, nullptr) == -1 ||
      event.flags & EV_ERROR) {
    return -1;
  }
  EV_SET(&event, fd, EVFILT_WRITE, EV_ADD, 0, 0, ud);
  if (kevent(d.fd, &event, 1, nullptr, 0, nullptr) == -1 ||
      event.flags & EV_ERROR) {
    EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    kevent(d.fd, &event, 1, nullptr, 0, nullptr);
    return -1;
  }
  EV_SET(&event, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, ud);
  if (kevent(d.fd, &event, 1, nullptr, 0, nullptr) == -1 ||
      event.flags & EV_ERROR) {
    poll_delete(d, fd);
    return -1;
  }
  return 0;
}

int32_t poll_enable(
  data_t &d, int32_t fd, connection::id_t conn_id, bool read_enable,
  bool write_enable) {
  if (conn_id > d.event_datas.size() || conn_id < 0 
      || d.fd == socket::kInvalidId) {
    return -1;
  }
  size_t index = conn_id - 1;
  auto ud = &d.event_datas[index];
  struct kevent event; 
  EV_SET(
    &event, fd, EVFILT_READ, read_enable ? EV_ENABLE : EV_DISABLE, 0, 0, ud);
  if (kevent(d.fd, &event, 1, nullptr, 0, nullptr) == -1 ||
      event.flags & EV_ERROR) {
    return -1;
  }
  EV_SET(
    &event, fd, EVFILT_WRITE, read_enable ? EV_ENABLE : EV_DISABLE, 0, 0, ud);
  if (kevent(d.fd, &event, 1, nullptr, 0, nullptr) == -1 ||
      event.flags & EV_ERROR) {
    return -1;
  }
  return 0;
}

int32_t poll_wait(data_t &d, int32_t timeout) {
  assert(d.events);
  assert(d.max_count > 0);
  if (timeout >= 0) {
    struct timespec ts{
      static_cast<decltype(ts.tv_sec)>(timeout / 1000),
      static_cast<decltype(ts.tv_nsec)>(timeout % 1000 * 1000 * 1000)
    };
    d.result_event_count = kevent(d.fd, nullptr, 0, d.events, d.max_count, &ts);
  } else {
    d.result_event_count =
      kevent(d.fd, nullptr, 0, d.events, d.max_count, nullptr);
  }
  d.event_index = 0;
  return d.result_event_count;
}

int32_t poll_destory(data_t &d) {
  if (d.fd == -1) return 0;
  plain::close(d.fd);
  safe_delete_array(d.events);
  d.fd = -1;
  return 0;
}

#endif

}

struct Kqueue::Impl {
#ifdef ENABLE_KQUEUE
  data_t data;
  std::mutex mutex;
#endif
};

Kqueue::Kqueue(
  const setting_t &setting,
  std::shared_ptr<concurrency::executor::Basic> executor) :
  Manager(setting, executor), impl_{std::make_unique<Impl>()} {
}

Kqueue::~Kqueue() {
#ifdef ENABLE_KQUEUE
  poll_destory(impl_->data);
#endif
}
  
bool Kqueue::prepare() noexcept {
#ifdef ENABLE_KQUEUE
  if (running()) return true;
  impl_->data.event_datas = std::vector<uint64_t>(setting_.max_count, 0);
  auto fd = poll_create(impl_->data, setting_.max_count);
  if (fd <= 0) {
    LOG_ERROR << setting_.name << " create error max_count: "
      << setting_.max_count << " fd: " << fd;
    return false;
  }
  if (listen_fd_ != socket::kInvalidId) {
    auto r = poll_add(impl_->data, listen_fd_, connection::kInvalidId);
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

bool Kqueue::work() noexcept {
#ifdef ENABLE_KQUEUE
  poll_wait(impl_->data, 0);
  if (impl_->data.result_event_count < 0) {
    LOG_ERROR << setting_.name << " error: " << impl_->data.result_event_count;
    return false;
  }
  handle_input();
  return true;
#else
  return false;
#endif
}

void Kqueue::off() noexcept {
#ifdef ENABLE_KQUEUE
  poll_destory(impl_->data);
#endif
}

bool Kqueue::sock_add(
  [[maybe_unused]] socket::id_t sock_id,
  [[maybe_unused]] connection::id_t conn_id) noexcept {
  assert(sock_id != socket::kInvalidId);
  assert(conn_id != connection::kInvalidId);
#ifdef ENABLE_KQUEUE
  if (poll_add(impl_->data, sock_id, conn_id) != 0 ||
      poll_enable(impl_->data, sock_id, conn_id, true, true) != 0) {
    LOG_ERROR << setting_.name << " error: " << strerror(errno);
  } else {
    return true;
  }
#endif
  return false;
}
  
bool Kqueue::sock_remove([[maybe_unused]] socket::id_t sock_id) noexcept {
  assert(sock_id >= 0);
  assert(sock_id != socket::kInvalidId);
#ifdef ENABLE_KQUEUE
  if (poll_delete(impl_->data, sock_id) != 0) {
    LOG_ERROR << setting_.name << "error: " << strerror(errno);
  } else {
    return true;
  }
#endif
  return false;
}

void Kqueue::handle_input() noexcept {
#ifdef ENABLE_KQUEUE
  size_t accept_count{0};
  auto &d = impl_->data;
  for (int32_t i = 0; i < d.result_event_count; ++i) {
    uint64_t ud =
      d.events[i].udata ? *reinterpret_cast<uint64_t *>(d.events[i].udata) : 0;
    auto sock_id = static_cast<socket::id_t>(get_highsection(ud));
    auto conn_id = static_cast<connection::id_t>(get_lowsection(ud));
    auto filter = d.events[i].filter;
    if (sock_id != socket::kInvalidId &&
        sock_id == listen_fd_ && accept_count < kOnceAcccpetCount) {
      ++accept_count;
      this->accept();
    } else if (sock_id != socket::kInvalidId && sock_id == ctrl_read_fd_) {
      recv_ctrl_cmd();
    } else if (filter == EVFILT_READ) {
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
    } else if (d.events[i].flags & EV_ERROR) {
      auto conn = get_conn(conn_id);
      if (conn) {
        LOG_ERROR << setting_.name << " kevent error";
        remove(conn_id);
        continue;
      }
    }
  }
#endif
}

}
