#include "pf/basic/logger.h"
#include "pf/basic/util.h"
#include "pf/net/connection/manager/select.h"

#if !(OS_UNIX && defined(PF_OPEN_EPOLL)) && \
  !(OS_WIN && defined(PF_OPEN_IOCP))

namespace pf_net {

namespace connection {

namespace manager {

Select::Select() {
  FD_ZERO(&readfds_[kSelectFull]);
  FD_ZERO(&writefds_[kSelectFull]);
  FD_ZERO(&exceptfds_[kSelectFull]);
  FD_ZERO(&readfds_[kSelectUse]);
  FD_ZERO(&writefds_[kSelectUse]);
  FD_ZERO(&exceptfds_[kSelectUse]);
  timeout_[kSelectFull].tv_sec = 0;
  timeout_[kSelectFull].tv_usec = 0;
  maxfd_ = minfd_ = SOCKET_INVALID;
}

Select::~Select() {
  //do nothing
}

bool Select::init(uint16_t connectionmax) {
  if (!Interface::init(connectionmax)) return false;
  if (listener_socket_id() != ID_INVALID) {
    FD_SET(listener_socket_id(), &readfds_[kSelectFull]);
    FD_SET(listener_socket_id(), &exceptfds_[kSelectFull]);
    minfd_ = maxfd_ = listener_socket_id();
  }
  timeout_[kSelectFull].tv_sec = 0;
  timeout_[kSelectFull].tv_usec = 0;
  return true;    
}

bool Select::select() {
	if (SOCKET_INVALID == minfd_ && SOCKET_INVALID == maxfd_)
    return true; //no connection
  timeout_[kSelectUse].tv_sec = timeout_[kSelectFull].tv_sec;
  timeout_[kSelectUse].tv_usec = timeout_[kSelectFull].tv_usec;
  readfds_[kSelectUse] = readfds_[kSelectFull];
  writefds_[kSelectUse] = writefds_[kSelectFull];
  exceptfds_[kSelectUse] = exceptfds_[kSelectFull];
  int32_t result = SOCKET_ERROR;
  try {
    result = socket::Basic::select(
        maxfd_ + 1,
        &readfds_[kSelectUse],
        &writefds_[kSelectUse],
        &exceptfds_[kSelectUse],
        &timeout_[kSelectUse]);
    Assert(result != SOCKET_ERROR);
  } catch(...) {
    FAST_ERRORLOG(NET_MODULENAME, 
                  "[net.connection.manager] (Select::select)"
                  " have error, result: %d", 
                  result);
  }
  return true;
}

bool Select::process_input() {
  if (SOCKET_INVALID == minfd_ && SOCKET_INVALID == maxfd_)
    return true; //no connection
  uint16_t i;
  //接受新连接的时候至少尝试两次，所以连接池里会多创建一个
  if (listener_socket_id() != SOCKET_INVALID && 
      FD_ISSET(listener_socket_id(), &readfds_[kSelectUse])) {
    for (i = 0; i < onestep_accept_; ++i) {
      if (!accept()) break;
    }
  }
  uint16_t _size = size();
  for (i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection::Basic *connection = nullptr;
    connection = pool_->get(connection_idset_[i]);
    Assert(connection);
    int32_t socket_id = connection->socket()->get_id();
    if (listener_socket_id() == socket_id) continue;
    if (FD_ISSET(socket_id, &readfds_[kSelectUse])) { //read information
      if (connection->socket()->error()) {
        remove(connection);
      } else {
        try {
          if (!connection->process_input()) { 
            remove(connection);
          } else {
            receive_bytes_ += connection->get_receive_bytes();
          }
        } catch(...) {
          remove(connection);
        }
      }//connection->socket()->error()
    }
  }
  return true;
}

bool Select::process_output() {
  if (SOCKET_INVALID == maxfd_ && SOCKET_INVALID == minfd_)
    return true;
  uint16_t i;
  uint16_t _size = size();
  for (i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection::Basic* connection = nullptr;
    connection = pool_->get(connection_idset_[i]);
    Assert(connection);
    int32_t socket_id = connection->socket()->get_id();
    if (listener_socket_id() == socket_id) continue;
    if (FD_ISSET(socket_id, &writefds_[kSelectUse])) {
      if (connection->socket()->error()) {
        remove(connection);
      } else {
        try {
          if (!connection->process_output()) { 
            remove(connection);
          } else {
            send_bytes_ += connection->get_send_bytes();
          }
        } catch(...) {
          remove(connection);
        }
      } //connection->socket()->error()
    }
  }
  return true;
}

bool Select::process_exception() {
  if (SOCKET_INVALID == minfd_ && SOCKET_INVALID == maxfd_)
    return true;
  uint16_t _size = size();
  connection::Basic *connection = nullptr;
  uint16_t i;
  for (i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection = pool_->get(connection_idset_[i]);
    Assert(connection);
    int32_t socket_id = connection->socket()->get_id();
    if (listener_socket_id() == socket_id) {
      Assert(false);
      continue;
    }
    if (FD_ISSET(socket_id, &exceptfds_[kSelectUse])) {
      remove(connection);
    }
  }
  return true;
}

bool Select::process_command() {
  if (SOCKET_INVALID == maxfd_ && SOCKET_INVALID == minfd_)
    return true;
  uint16_t i;
  uint16_t _size = size();
  for (i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection::Basic* connection = nullptr;
    connection = pool_->get(connection_idset_[i]);
    Assert(connection);
    int32_t socket_id = connection->socket()->get_id();
    if (listener_socket_id() == socket_id) continue;
    if (connection->socket()->error()) {
      remove(connection);
    } else { //connection is ok
      try {
        if (!connection->process_command()) 
          remove(connection);
      } catch(...) {
        remove(connection);
      }
    } //connection->socket()->error()
  }
  return true;
}

bool Select::socket_add(int32_t socketid, int16_t) {
  if (fdsize_ > FD_SETSIZE) {
    Assert(false);
    return false;
  }
  Assert(SOCKET_INVALID != socketid);
  minfd_ = SOCKET_INVALID == minfd_ ? socketid : min(socketid, minfd_);
  maxfd_ = SOCKET_INVALID == maxfd_ ? socketid : max(socketid, maxfd_);
  FD_SET(socketid, &readfds_[kSelectFull]);
  FD_SET(socketid, &writefds_[kSelectFull]);
  FD_SET(socketid, &exceptfds_[kSelectFull]);
  ++fdsize_;
  return true;
}

bool Select::socket_remove(int32_t socketid) {
  if (SOCKET_INVALID == socketid) {
    SLOW_ERRORLOG(NET_MODULENAME,
                  "[net.connection.manager] (Select::socket_remove) error!"
                  " SOCKET_INVALID == socket_id");
    return false;
  }
  connection::Basic *connection = nullptr;
  int32_t _listener_socket_id = listener_socket_id();
  uint16_t i;
  if (is_service())
    Assert(minfd_ != SOCKET_INVALID || maxfd_ != SOCKET_INVALID);
  Assert(fdsize_ > 0);
  if (socketid == minfd_) { //the first connection
    int32_t socketid_max = maxfd_;
    uint16_t _size = size();
    for (i = 0; i < _size; ++i) {
      if (ID_INVALID == connection_idset_[i]) continue;
      connection = pool_->get(connection_idset_[i]);
      Assert(connection);
      if (nullptr == connection) continue;
      int32_t _socketid = connection->socket()->get_id();
      if (socketid == _socketid || SOCKET_INVALID == _socketid) continue;
      if (socketid_max < _socketid) socketid_max = _socketid;
    }
    if (minfd_ == maxfd_) {
      minfd_ = maxfd_ = SOCKET_INVALID;
    } else {
      minfd_ = socketid_max > listener_socket_id() ? minfd_ : socketid_max;
    }
  } else if (socketid == maxfd_) { //
    int32_t socketid_min = minfd_;
    uint16_t _size = size();
    for (i = 0; i < _size; ++i) {
      if (ID_INVALID == connection_idset_[i]) continue;
      connection = pool_->get(connection_idset_[i]);
      Assert(connection);
      if (nullptr == connection) continue;
      int32_t _socketid = connection->socket()->get_id();
      if (socketid == _socketid || SOCKET_INVALID == _socketid) continue;
      if (socketid_min > _socketid) socketid_min = _socketid;
    }
    if (minfd_ == maxfd_) {
      minfd_ = maxfd_ = SOCKET_INVALID;
    } else {
      maxfd_ = 
        socketid_min < _listener_socket_id ? _listener_socket_id : socketid_min;
    }
  }
  FD_CLR(static_cast<uint32_t>(socketid), &readfds_[kSelectFull]);
  FD_CLR(static_cast<uint32_t>(socketid), &readfds_[kSelectUse]);
  FD_CLR(static_cast<uint32_t>(socketid), &writefds_[kSelectFull]);
  FD_CLR(static_cast<uint32_t>(socketid), &writefds_[kSelectUse]);
  FD_CLR(static_cast<uint32_t>(socketid), &exceptfds_[kSelectFull]);
  FD_CLR(static_cast<uint32_t>(socketid), &exceptfds_[kSelectUse]);
  --fdsize_;
  Assert(fdsize_ >= 0);
  return true;
}

bool Select::heartbeat(uint64_t time) {
  bool result = Interface::heartbeat(time);
  return result;
}

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif
