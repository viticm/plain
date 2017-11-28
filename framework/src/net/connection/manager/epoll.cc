#include "pf/basic/logger.h"
#include "pf/basic/util.h"
#include "pf/net/connection/manager/epoll.h"

#if OS_UNIX && defined(PF_OPEN_EPOLL)

namespace pf_net {

namespace connection {

namespace manager {

Epoll::Epoll() {
  polldata_.fd = ID_INVALID;
  polldata_.maxcount = 0;
  polldata_.result_eventcount = 0;
  polldata_.event_index = 0;
  polldata_.events = nullptr;
}

Epoll::~Epoll() {
  poll_destory(polldata_);
}

bool Epoll::init(uint16_t connectionmax) {
  if (!poll_set_max_size(connectionmax)) return false;
  if (!Interface::init(connectionmax)) return false;
  return true;
}

bool Epoll::select() {
  int32_t result = SOCKET_ERROR;
  try {
    poll_wait(polldata_, 0);
    if (polldata_.result_eventcount > polldata_.maxcount || 
        polldata_.result_eventcount < 0) {
      char message[128] = {0};
      snprintf(message, 
               sizeof(message) - 1, 
               "Epoll::select error, result event count: %d",
               polldata_.result_eventcount);
      AssertEx(false, message);
    }
  } catch(...) {
    FAST_ERRORLOG(NET_MODULENAME, 
                  "[net.connection.manager] (Epoll::select)"
                  " have error, result: %d", 
                  result);
  }
  return true;
}

bool Epoll::poll_set_max_size(uint16_t max_size) {
  if (polldata_.fd > 0) return true;
  bool result = poll_create(polldata_, max_size) > 0 ? true : false;
  if (!result) return false;
  if (is_service()) {
    if (ID_INVALID == listener_socket_id()) return false;
    poll_add(polldata_, listener_socket_id(), EPOLLIN, ID_INVALID);
  }
  return true;
}

bool Epoll::socket_add(int32_t socket_id, int16_t connection_id) {
  if (fdsize_ > polldata_.maxcount) {
    Assert(false);
    return false;
  }
  Assert(SOCKET_INVALID != socket_id);
  if (poll_add(polldata_, socket_id, EPOLLIN | EPOLLET, connection_id) != 0) {
    SLOW_ERRORLOG(NET_MODULENAME, 
                  "[net.connection.manager] (Epoll::socket_add)"
                  " error, message: %s", 
                  strerror(errno));
    return false;
  }
  ++fdsize_;
  return true;
}

bool Epoll::socket_remove(int32_t socket_id) {
  if (SOCKET_INVALID == socket_id) {
    SLOW_ERRORLOG(NET_MODULENAME,
                  "[net.connection.manager] (Epoll::socket_remove) error!"
                  " SOCKET_INVALID == socket_id");
    return false;
  }
  if (socket_id <= 0) {
    SLOW_ERRORLOG(NET_MODULENAME,
                  "[net.connection.manager] (Epoll::socket_remove) error!"
                  "socketid(%d) <= 0", socket_id);
    return false;
  }
  if (poll_delete(polldata_, socket_id) != 0) {
    SLOW_ERRORLOG(NET_MODULENAME,
                  "[net.connection.manager] (Epoll::socket_remove) error!"
                  " message: %s",
                  strerror(errno));
    return false;
  }
  --fdsize_;
  Assert(fdsize_ >= 0);
  return true;
}

bool Epoll::process_input() {
  using namespace pf_basic;
  uint16_t i;
  uint16_t accept_count{0};
  for (i = 0; i < polldata_.result_eventcount; ++i) {
    //接受新连接的时候至少尝试两次，所以连接池里会多创建一个
    int32_t socket_id = static_cast<int32_t>(
        util::get_highsection(polldata_.events[i].data.u64));
    int16_t connection_id = static_cast<int16_t>(
        util::get_lowsection(polldata_.events[i].data.u64));
    if (socket_id != SOCKET_INVALID && 
        socket_id == listener_socket_id() && 
        accept_count < onestep_accept_ ) {
      accept();
      ++accept_count;
    } else if (polldata_.events[i].events & EPOLLIN) {
      connection::Basic *connection = nullptr;
      if (ID_INVALID == connection_id) {
        SLOW_WARNINGLOG(NET_MODULENAME, 
                        "[net.connection.manager] (Epoll::processinput)"
                        " ID_INVALID == connectionid");
        continue;
      }
      connection = get(connection_id);
      if (nullptr == connection) {
        SLOW_WARNINGLOG(NET_MODULENAME, 
                        "[net.connection.manager] (Epoll::processinput)"
                        " nullptr == connection, id: %d", connection_id);
        continue;
      }
      if (connection->is_disconnect()) continue;
      //int32_t socket_id = connection->socket()->get_id();
      if (SOCKET_INVALID == socket_id) {
        SLOW_ERRORLOG(NET_MODULENAME,
                      "[net.connection.manager] (Epoll::processinput)"
                      " error! socket_id == SOCKET_INVALID, connectionid: %d",
                      connection_id);
        return false;
      }
      if (connection->socket()->error()) {
        pf_basic::io_cerr("connection->socket()->error()");
        remove(connection);
      } else {
        try {
          if (!connection->process_input()) { 
            pf_basic::io_cerr("!connection->process_input()");
            remove(connection);
          } else {
            receive_bytes_ += connection->get_receive_bytes();
          }
        } catch(...) {
          pf_basic::io_cerr("connection catch");
          remove(connection);
        }
      }
    } //handle the epoll input event
  }
  return true;
}

bool Epoll::process_output() {
  uint16_t i;
  uint16_t _size = size();
  for (i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection::Basic* connection = nullptr;
    connection = pool_->get(connection_idset_[i]);
    Assert(connection);
    if (connection->socket()->error()) {
      char msg[1024]{0};
      connection->socket()->get_last_error_message(msg, sizeof(msg) - 1);
      pf_basic::io_cerr("msg: %s", msg);
      throw 1;
      pf_basic::io_cerr("connection->socket()->error() 1");
      remove(connection);
    } else {
      try {
        if (!connection->process_output()) { 
          pf_basic::io_cerr("!connection->process_output()");
          remove(connection);
        } else {
          send_bytes_ += connection->get_send_bytes();
        }
      } catch(...) {
        remove(connection);
      }
    } //connection->socket()->error()
  }
  return true;
}

bool Epoll::process_exception() {
  return true;
}

bool Epoll::process_command() {
  uint16_t i;
  uint16_t _size = size();
  for (i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection::Basic* connection = nullptr;
    connection = pool_->get(connection_idset_[i]);
    Assert(connection);
    if (connection->is_disconnect()) continue;
    int32_t socket_id = connection->socket()->get_id();
    if (listener_socket_id() == socket_id) continue;
    if (connection->socket()->error()) {
      remove(connection);
    } else { //connection is ok
      try {
        if (!connection->process_command()) {
          remove(connection);
        }
      } catch(...) {
        remove(connection);
      }
    } //connection->getsocket()->iserror()
  }
  return true;
}

bool Epoll::heartbeat(uint32_t time) {
  bool result = Interface::heartbeat(time);
  return result;
}

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif
