#include "plain/net/connection/select.h"
#include <cassert>
#include <set>
#include "plain/basic/logger.h"
#include "plain/basic/utility.h"
#include "plain/file/api.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/connection/basic.h"
#include "plain/net/connection/detail/coroutine.h"
#include "plain/net/socket/basic.h"
#include "plain/net/socket/api.h"


using plain::net::connection::Select;

struct Select::Impl {
  
  static constexpr size_t kOnceAcccpetCount{64};
  
  struct fd_info_t {
    fd_set full;
    fd_set use;
  };

  struct timeout_info_t {
    timeval full;
    timeval use;
  };

  fd_info_t read_fds;
  fd_info_t write_fds;
  fd_info_t except_fds;
  timeout_info_t timeout;
  int32_t fd_size{0};
  socket::id_t max_fd{socket::kInvalidSocket};
  std::set<socket::id_t> valid_fds;
  std::condition_variable cv;
  std::mutex mutex;
  bool running{false};
  int32_t select_result{-1};

  // flag: true add false remove
  void change_vaild_fd(socket::id_t id, bool flag) noexcept {
    assert(id != socket::kInvalidSocket);
    if (flag) {
      valid_fds.emplace(id);
    } else {
      auto it = valid_fds.find(id);
      if (it != valid_fds.end()) valid_fds.erase(it);
    }
    max_fd = valid_fds.empty() ? socket::kInvalidSocket : *valid_fds.rbegin();
  }

  void select() noexcept;
};

void Select::Impl::select() noexcept {
  if (max_fd == socket::kInvalidSocket) return;
  timeout.use.tv_sec = timeout.full.tv_sec;
  timeout.use.tv_usec = timeout.full.tv_usec;
  read_fds.use = read_fds.full;
  write_fds.use = write_fds.full;
  except_fds.use = except_fds.full;
  select_result = socket::select(
    max_fd + 1, &read_fds.use, &write_fds.use, &except_fds.use, &timeout.use);
}

Select::Select(const setting_t &setting) :
  Manager(setting), impl_{std::make_shared<Impl>()} {

}
  
Select::Select(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  Manager(std::forward<decltype(executor)>(executor), setting),
  impl_{std::make_shared<Impl>()} {

}

Select::~Select() = default;

bool Select::work() noexcept {
  impl_->change_vaild_fd(listen_fd_, true);
  impl_->running = true;
  work_recurrence();
  return true;
}
  
void Select::off() noexcept {
  impl_->running = false;
}

plain::net::connection::detail::Task Select::work_recurrence() noexcept {
  auto func = [self = impl_](
    concurrency::coroutine_handle<detail::Task::promise_type> handle) {
    if (self->max_fd == socket::kInvalidSocket) return false;
    std::unique_lock<std::mutex> lock{self->mutex};
    self->cv.wait(lock, [self] {
      self->select();
      return self->select_result != 0 || !self->running;
    });
    return true;
  };
  co_await detail::Awaitable{func};
  if (impl_->select_result < 0) {
    LOG_ERROR << "work_recurrence error: " << impl_->select_result;
  } else {
    handle_io();
  }
  if (impl_->running) work_recurrence();
}
  
void Select::handle_io() noexcept {
  if (!impl_->running) return;
  if (listen_fd_ != socket::kInvalidSocket &&
      FD_ISSET(listen_fd_, &impl_->read_fds.use)) {
    for (size_t i = 0; i < Impl::kOnceAcccpetCount; ++i) {
      if (!accept()) break;
    }
  }
  try {
    foreach([this, listen_fd = listen_fd_](std::shared_ptr<Basic> conn){
      if (!impl_->running) return;
      auto id = conn->socket()->id();
      if (id == socket::kInvalidSocket || id == listen_fd) return;
      if (FD_ISSET(id, &impl_->except_fds.use)) {
        LOG_ERROR << "connection has except: " << conn->id();
        this->remove(conn->id());
        return;
      } else if (FD_ISSET(id, &impl_->read_fds.use) ||
        FD_ISSET(id, &impl_->write_fds.use)) {
        LOG_DEBUG << "hanle io: " << conn->id();
        conn->enqueue_work();
      }
      
    });
  } catch(...) {
    LOG_ERROR << "handle io except";
  }
}
  
bool Select::sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept {
  assert(impl_->fd_size < FD_SETSIZE);
  assert(sock_id != socket::kInvalidSocket);
  assert(conn_id != connection::kInvalidId);

  FD_SET(sock_id, &impl_->read_fds.full);
  FD_SET(sock_id, &impl_->write_fds.full);
  FD_SET(sock_id, &impl_->except_fds.full);
  impl_->change_vaild_fd(sock_id, true);
  ++impl_->fd_size;
  return true;
}
  
bool Select::sock_remove(socket::id_t sock_id) noexcept {
  assert(sock_id != socket::kInvalidSocket);
  auto id = static_cast<uint32_t>(sock_id);
  FD_CLR(id, &impl_->read_fds.full);
  FD_CLR(id, &impl_->read_fds.use);
  FD_CLR(id, &impl_->write_fds.full);
  FD_CLR(id, &impl_->write_fds.use);
  FD_CLR(id, &impl_->except_fds.full);
  FD_CLR(id, &impl_->except_fds.use);
  impl_->change_vaild_fd(sock_id, false);
  return true;
}
