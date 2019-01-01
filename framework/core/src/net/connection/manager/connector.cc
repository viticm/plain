#include "pf/basic/logger.h"
#include "pf/sys/assert.h"
#include "pf/net/socket/basic.h"
#include "pf/net/connection/manager/connector.h"

using namespace pf_net::connection::manager;

bool Connector::init(uint16_t _max_size) {
  /* Some bug with no service in deamon, interim resolvent ??? */
  socket::Basic socket; socket.create();
  /* Interim resolvent ??? */
  return Basic::init(_max_size);
}

pf_net::connection::Basic *Connector::connect(const char *ip, uint16_t port) {
  if (!checkpool()) return nullptr;
  pf_net::connection::Basic *connection = pool_->create();
  if (is_null(connection)) return nullptr;
  if (!connection->init(protocol())) {
    return nullptr;
  }
  connection->clear();
  pf_net::socket::Basic *socket = connection->socket();
  bool result = false;
  uint8_t step = 0;
  bool _remove = false;
  try {
    result = socket->is_valid() ? true : socket->create();
    if (!result) {
      step = 1;
      goto EXCEPTION;
    }
    result = socket->connect(ip, port);
    if (!result) {
      step = 2;
      goto EXCEPTION;
    }
    result = socket->set_nonblocking();
    if (!result) {
      step = 3;
      goto EXCEPTION;
    }
    result = socket->set_linger(0);
    if (!result) {
      step = 4;
      goto EXCEPTION;
    }
  } catch (...) {
    step = 5;
    goto EXCEPTION;
  }
  result = add(connection);
  if (!result) {
    step = 6;
    goto EXCEPTION;
  }
  _remove = true;
  connection->set_disconnect(false); //Success.
  SLOW_LOG(NET_MODULENAME,
           "[net.connection.manager] (Connector::connect) success!"
           " ip: %s, port: %d",
           ip,
           port);
  return connection;
EXCEPTION:
  SLOW_WARNINGLOG(NET_MODULENAME,
                  "[net.connection.manager] (Connector::connect) failed!"
                  " ip: %s, port: %d, step: %d",
                  ip,
                  port,
                  step);
  if (_remove) {
    remove(connection);
  } else {
    pool_->remove(connection->get_id());
  }
  return nullptr;
}

pf_net::connection::Basic *
Connector::group_connect(const char *ip, uint16_t port) {
  if (!checkpool()) return nullptr;
  uint8_t step = 0;
  bool result = false;
  bool _remove = false;
  bool closesocket = false;
  pf_net::connection::Basic *connection = pool_->create(true);
  if (nullptr == connection) return nullptr;
  if (!connection->init(protocol())) return nullptr;
  pf_net::socket::Basic *socket = connection->socket();
  try {
    result = socket->create();
    if (!result) {
      step = 1;
      goto EXCEPTION;
    }
    result = socket->set_nonblocking();
    if (!result) {
      step = 2;
      goto EXCEPTION;
    }
    result = socket->connect(ip, port);
    if (!result) {
      step = 3;
      struct timeval tm;
      fd_set readset, writeset;
      tm.tv_sec = 10;
      tm.tv_usec = 0;
      FD_ZERO(&readset);
      FD_SET(socket->get_id(), &readset);
      writeset = readset;
      int32_t _result = 0;
      _result = 
        socket->select(socket->get_id() + 1, &readset, &writeset, nullptr, &tm);
      closesocket = true;
      if (0 == _result) { //连接超时
        step = 4;
        goto EXCEPTION;
      }
      //如果描述符变为可读或可写，就调用get_sockopt取得套接字的待处理错误
      if (FD_ISSET(socket->get_id(), &readset) || 
          FD_ISSET(socket->get_id(), &writeset)) {
        uint32_t length = sizeof(_result);
        if (!pf_net::socket::api::getsockopt_exb(
              socket->get_id(), SOL_SOCKET, SO_ERROR, &result, &length)) {
          step = 5;
          goto EXCEPTION;
        }
        if (_result != 0) {
          step = 6;
          goto EXCEPTION;
        }
      }
    }
    result = socket->set_nonblocking();
    if (!result) {
      step = 7;
      goto EXCEPTION;
    }
    result = socket->set_linger(0);
    if (!result) {
      step = 8;
      goto EXCEPTION;
    }
  } catch (...) {
    step = 9;
    goto EXCEPTION;
  }
  result = add(connection); //加入到管理器
  if (!result) {
    step = 10;
    goto EXCEPTION;
  }
  connection->set_disconnect(false); //Success.
  _remove = true;
  SLOW_LOG(NET_MODULENAME,
           "[net.connection.manager] (Connector::group_connect) success!"
           " ip: %s, port: %d",
           ip,
           port);
  return connection;
EXCEPTION:
  SLOW_WARNINGLOG(
      NET_MODULENAME,
      "[net.connection.manager] (Connector::group_connect) failed!"
      " ip: %s, port: %d, step: %d",
      ip,
      port,
      step);
  if (closesocket) socket->close();
  if (_remove) {
    remove(connection);
  } else {
    pool_->remove(connection->get_id());
  }
  return nullptr;
}
