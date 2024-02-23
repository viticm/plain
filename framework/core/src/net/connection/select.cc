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
    fd_info_t() {
      std::memset(&full, 0, sizeof(full));
      std::memset(&use, 0, sizeof(use));
    }
    fd_set full;
    fd_set use;
  };

  fd_info_t read_fds;
  fd_info_t write_fds;
  fd_info_t except_fds;
  timeval timeout;
  int32_t fd_size{0};
  socket::id_t max_fd{socket::kInvalidId};
  std::set<socket::id_t> valid_fds;
  std::mutex mutex;
  int32_t select_timeout{10}; // The select timeout(ms).
  int32_t select_result{0};

  // flag: true add false remove
  void change_vaild_fd(socket::id_t id, bool flag) noexcept {
    assert(id != socket::kInvalidId);
    if (flag) {
      valid_fds.emplace(id);
    } else {
      if (valid_fds.empty()) return;
      auto it = valid_fds.find(id);
      if (it != valid_fds.end()) valid_fds.erase(it);
    }
    max_fd = valid_fds.empty() ? socket::kInvalidId : *valid_fds.rbegin();
  }

  void select() noexcept;
};

void Select::Impl::select() noexcept {
  if (max_fd == socket::kInvalidId) return;
  read_fds.use = read_fds.full;
  write_fds.use = write_fds.full;
  except_fds.use = except_fds.full;
  if (select_timeout >= 0) {
    select_result = socket::select(
      max_fd + 1, &read_fds.use, &write_fds.use, &except_fds.use, &timeout);
  } else {
    select_result = socket::select(
      max_fd + 1, &read_fds.use, &write_fds.use, &except_fds.use, nullptr);
  }
  // std::cout << "select: " << select_result << std::endl;
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

bool Select::prepare() noexcept {
  if (listen_fd_ != socket::kInvalidId) {
    FD_SET(listen_fd_, &impl_->read_fds.full);
    FD_SET(listen_fd_, &impl_->except_fds.full);
    impl_->change_vaild_fd(listen_fd_, true);
    impl_->select_timeout = -1;
  } else if (ctrl_read_fd_ != socket::kInvalidId) {
    impl_->select_timeout = -1;
  } else {
    impl_->timeout.tv_sec = (impl_->select_timeout / 1000);
    impl_->timeout.tv_usec = (impl_->select_timeout % 1000 * 1000);
  }
  return true;
}

bool Select::work() noexcept {
  impl_->select();
  bool r{true};
  if (impl_->select_result < 0) {
    r = false;
    LOG_ERROR << setting_.name << " error: " << impl_->select_result;
  } else {
    handle_io();
  }
  return r;
}
  
void Select::off() noexcept {

}
 
void Select::handle_io() noexcept {
  if (listen_fd_ != socket::kInvalidId &&
      FD_ISSET(listen_fd_, &impl_->read_fds.use)) {
    for (size_t i = 0; i < Impl::kOnceAcccpetCount; ++i) {
      if (!accept()) break;
    }
  }
  if (ctrl_read_fd_ != socket::kInvalidId &&
      FD_ISSET(ctrl_read_fd_, &impl_->read_fds.use)) {
    recv_ctrl_cmd();
  }
  try {
    foreach([this, listen_fd = listen_fd_](std::shared_ptr<Basic> conn){
      auto id = conn->socket()->id();
      if (FD_ISSET(id, &impl_->except_fds.use)) {
        LOG_ERROR << setting_.name << " connection has except: " << conn->id();
        this->remove(conn->id());
        return;
      } else if (FD_ISSET(id, &impl_->read_fds.use)) {
        conn->enqueue_work(WorkFlag::Input);
      } else if (FD_ISSET(id, &impl_->write_fds.use)) { // output ?

      }
      
    });
  } catch(...) {
    LOG_ERROR << setting_.name << " handle io except";
  }
}
  
bool Select::sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept {
  assert(impl_->fd_size < FD_SETSIZE);
  assert(sock_id != socket::kInvalidId);
  assert(conn_id != connection::kInvalidId);

  FD_SET(sock_id, &impl_->read_fds.full);
  FD_SET(sock_id, &impl_->write_fds.full);
  FD_SET(sock_id, &impl_->except_fds.full);
  impl_->change_vaild_fd(sock_id, true);
  ++impl_->fd_size;
  return true;
}
  
bool Select::sock_remove(socket::id_t sock_id) noexcept {
  assert(sock_id != socket::kInvalidId);
  auto id = static_cast<uint32_t>(sock_id);
  FD_CLR(id, &impl_->read_fds.full);
  FD_CLR(id, &impl_->read_fds.use);
  FD_CLR(id, &impl_->write_fds.full);
  FD_CLR(id, &impl_->write_fds.use);
  FD_CLR(id, &impl_->except_fds.full);
  FD_CLR(id, &impl_->except_fds.use);
  impl_->change_vaild_fd(sock_id, false);
  --impl_->fd_size;
  return true;
}
