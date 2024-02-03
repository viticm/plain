#include "plain/net/socket/listener.h"
#include "plain/net/socket/api.h"
#include "plain/net/socket/basic.h"
#include "plain/basic/logger.h"

using plain::net::socket::Listener;

Listener::Listener() : socket_{std::make_unique<plain::net::socket::Basic>()} {

}

Listener::~Listener() = default;

bool Listener::init(const Address &addr, uint32_t backlog) {
  if (!socket_->close()) return false;
  if (!socket_->create(addr.family(), SOCK_STREAM, 0)) {
    LOG_ERROR << "can't create: " << get_last_error();
    return false;
  }
  if (!socket_->set_reuse_addr()) {
    LOG_ERROR << "can't set reuse addr: " << get_last_error();
    return false;
  }
  if (!socket_->bind(addr)) {
    LOG_ERROR << "can't bind addr: " << addr.text()
      << " error: " << get_last_error();
    return false;
  }
  if (!socket_->listen(backlog)) {
    LOG_ERROR << "can't listen: " << backlog << " error: " << get_last_error();
    return false;
  }
  if (!socket_->set_nonblocking()) {
    LOG_ERROR << "can't set nonblocking" << " error: " << get_last_error();
    return false;
  }
  return true;
}

void Listener::close() {
  socket_->close();
}

bool Listener::accept(std::shared_ptr<Basic> socket) {
  if (!socket) return false;
  if (!socket_->valid()) return false;
  id_t id = socket_->accept();
  if (id == kInvalidId) return false;
  return socket->set_id(id);
}

uint32_t Listener::get_linger() const noexcept {
  return socket_->get_linger();
}

bool Listener::set_linger(uint32_t lingertime) noexcept {
  return socket_->set_linger(lingertime);
}

bool Listener::is_nonblocking() const noexcept {
  return socket_->is_nonblocking();
}
  
bool Listener::set_nonblocking(bool on) noexcept {
  return socket_->set_nonblocking(on);
}
  
uint32_t Listener::get_recv_size() const noexcept {
  return socket_->get_recv_size();
}
  
void Listener::set_recv_size(uint32_t size) noexcept {
  socket_->set_recv_size(size);
}

uint32_t Listener::get_send_size() const noexcept {
  return socket_->get_send_size();
}
  
void Listener::set_send_size(uint32_t size) noexcept {
  socket_->set_send_size(size);
}
  
plain::net::socket::id_t Listener::id() const noexcept {
  return socket_->id();
}
  
plain::net::Address Listener::address() const noexcept {
  return socket_->address();
}
