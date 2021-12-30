#include "pf/basic/logger.h"
#include "pf/basic/time_manager.h"
#include "pf/sys/assert.h"
#include "pf/net/socket/listener.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/connection/manager/listener.h"

using namespace pf_net::connection::manager;

Listener::Listener() :
  listener_socket_{nullptr},
  safe_encrypt_str_{""} {
  //do nothing
}

Listener::~Listener() {
  //do nothing
}

bool Listener::init(uint16_t _max_size, uint16_t _port, const std::string &ip) {
  if (is_ready()) return true;
  std::unique_ptr<socket::Listener> 
    pointer{new socket::Listener()};
  if (is_null(pointer)) return false;
  listener_socket_ = std::move(pointer);
  if (!listener_socket_->init(_port, ip)) return false;
  listener_socket_->set_nonblocking();
  Assert(listener_socket_->get_id() != SOCKET_INVALID);
  return Basic::init(_max_size);
}

pf_net::connection::Basic *Listener::accept() {

  // Not accept connection when app stop.
  if (GLOBALS["app.status"] == kAppStatusStop) return nullptr;
  
  uint32_t step = 0;
  bool result = false;
  pf_net::connection::Basic *newconnection{nullptr};
  newconnection = pool_->create();
  if (is_null(newconnection)) { /* When pool full then will close new socket. */
    socket::Basic socket;
    if (listener_socket_->accept(&socket)) {
      socket.close();
    }
    static uint64_t checktime{0};
    auto _tick = TIME_MANAGER_POINTER->get_tickcount();
    if (0 == checktime || _tick - checktime >= 600000) {
      SLOW_WARNINGLOG(NET_MODULENAME, 
                      "[net.connection.manager] (Listener::accept)"
                      " can't accept new connection");
      checktime = _tick;
    }
    return nullptr;
  }
  step = 5;
  newconnection->init(protocol());
  newconnection->clear();
  int32_t socketid = SOCKET_INVALID;
  step = 10;
  try {
    //accept client socket
    result = listener_socket_->accept(newconnection->socket());
    if (!result) {
      step = 15;
      goto EXCEPTION;
    }
  } catch(...) {
    step += 1000;
    goto EXCEPTION;
  }
  try {
    step = 30;
    socketid = newconnection->socket()->get_id();
    if (SOCKET_INVALID == socketid) {
      Assert(false);
      goto EXCEPTION;
    }
    step = 40;
    result = newconnection->socket()->set_nonblocking();
    if (!result) {
      Assert(false);
      goto EXCEPTION;
    }
    step = 50;
    if (newconnection->socket()->error()) {
      Assert(false);
      goto EXCEPTION;
    }
    step = 60;
    result = newconnection->socket()->set_linger(0);
    if (!result) {
      Assert(false);
      goto EXCEPTION;
    }
    step = 70;
    try {
      result = add(newconnection);
      if (!result) {
        Assert(false);
        goto EXCEPTION;
      }
    } catch(...) {
      step += 10000;
      goto EXCEPTION;
    }
  } catch(...) {
    step += 100000;
  }
#ifdef _DEBUG
  FAST_LOG(NET_MODULENAME,
           "[net.connection.manager] (Listener::accept)"
           " host: %s id: %d socketid: %d",
           newconnection->socket()->host(),
           newconnection->get_id(),
           newconnection->socket()->get_id());
#endif
  return newconnection;
EXCEPTION:
  newconnection->clear();
  pool_->remove(newconnection->get_id());
  return nullptr;
}

void Listener::on_connect(connection::Basic *connection) {
  if (safe_encrypt_str_ != "")
    connection->set_safe_encrypt_time(TIME_MANAGER_POINTER->get_ctime());
  connection->set_listener(this);
}
