#include "pf/basic/string.h"
#include "pf/basic/io.tcc"
#include "pf/net/socket/listener.h"

namespace pf_net {

namespace socket {

Listener::Listener(uint16_t _port, const std::string &ip, uint32_t backlog) {
  using namespace pf_basic;
  bool result = false;
  std::unique_ptr< Basic > __socket(new pf_net::socket::Basic());
  socket_ = std::move(__socket);
  if (nullptr == socket_) { //memory not enough
    io_cerr("[net.socket] (Listener::Listener)"
            " new pap_common_net::socket::Base() failed,"
            " errorcode: %d",
            socket_->get_last_error_code());
    throw 1;
  }
  result = socket_->create();
  if (false == result) {
    io_cerr("[net.socket] (Listener::Listener)"
            " socket_->create() failed, errorcode: %d",
            socket_->get_last_error_code()); 
    throw 1;
  }
  result = socket_->set_reuseaddr();
  if (false == result) {
    io_cerr("[net.socket] (Listener::Listener)"
            " socket_->set_reuseaddr() failed, errorcode: %d",
            socket_->get_last_error_code());
    throw 1;
  }
  result = socket_->bind(_port, ip.c_str());
  if (false == result) {
    io_cerr("[net.socket] (Listener::Listener)"
            " socket_->bind(%d, %s) failed, errorcode: %d", 
            _port,
            ip.c_str(),
            socket_->get_last_error_code());
    throw 1;
  }
  result = socket_->listen(backlog);
  if (false == result) {
    io_cerr("[net.socket] (Listener::Listener)"
            " socket_->listen(%d) failed, errorcode: %d",
            backlog,
            socket_->get_last_error_code());
    throw 1;
  }
}

Listener::~Listener() {
  if (socket_ != nullptr) {
    socket_->close();
  }
}

void Listener::close() {
  if (socket_ != nullptr) socket_->close();
}

bool Listener::accept(pf_net::socket::Basic *socket) {
  using namespace pf_basic;
  if (nullptr == socket) return false;
  struct sockaddr_in accept_sockaddr_in;
  socket->close();
  socket->set_id(socket_->accept(&accept_sockaddr_in));
  if (SOCKET_INVALID == socket->get_id()) return false;
  socket->set_port(ntohs(accept_sockaddr_in.sin_port));
  socket->set_host(inet_ntoa(accept_sockaddr_in.sin_addr));
  return true;
}

uint32_t Listener::get_linger() const {
  uint32_t linger;
  linger = socket_->get_linger();
  return linger;
}

bool Listener::set_linger(uint32_t lingertime) {
  bool result = false;
  result = socket_->set_linger(lingertime);
  return result;
}

bool Listener::is_nonblocking() const {
  bool result = false;
  result = socket_->is_nonblocking();
  return result;
}

bool Listener::set_nonblocking(bool on) {
  return socket_->set_nonblocking(on);
}

uint32_t Listener::get_receive_buffer_size() const {
  uint32_t result = 0;
  result = socket_->get_receive_buffer_size();
  return result;
}

bool Listener::set_receive_buffer_size(uint32_t size) {
  bool result = false;
  result = socket_->set_receive_buffer_size(size);
  return result;
}

} //namespace socket

} //namespace pf_net
