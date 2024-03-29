#include "plain/net/connector.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/connection/manager.h"
#include "plain/net/connection/basic.h"
#include "plain/net/socket/api.h"
#include "plain/net/socket/basic.h"
#include "plain/net/utility.h"

using plain::net::Connector;

struct Connector::Impl {
  std::shared_ptr<connection::Manager> manager;
};

  
Connector::Connector(
  const setting_t &setting,
  std::shared_ptr<concurrency::executor::Basic> executor) :
  impl_{std::make_unique<Impl>()} {
  impl_->manager = make_manager(setting, executor);
  assert(impl_->manager);
}

Connector::~Connector() {
  // Manager must stoped.
  if (impl_->manager->running()) impl_->manager->stop();
}
  
bool Connector::start() {
  return impl_->manager->start();
}
  
void Connector::stop() {
  impl_->manager->stop();
}

std::shared_ptr<plain::net::connection::Basic>
Connector::connect(
  std::string_view address,
  std::function<bool(connection::Basic *)> init_func,
  const std::chrono::milliseconds &timeout,
  socket::Type sock_type) noexcept {
  return connect_impl(address, 0, init_func, timeout, sock_type);
}
 
std::shared_ptr<plain::net::connection::Basic>
Connector::connect(
  std::string_view ip, uint16_t port,
  std::function<bool(connection::Basic *)> init_func,
  const std::chrono::milliseconds &timeout,
  socket::Type sock_type) noexcept {
  return connect_impl(ip, port, init_func, timeout, sock_type);
}

void Connector::set_codec(const stream::codec_t &codec) noexcept {
  impl_->manager->set_codec(codec);
}
  
const plain::net::stream::codec_t &Connector::codec() const noexcept {
  return impl_->manager->codec();
}
  
void Connector::set_dispatcher(packet::dispatch_func func) noexcept {
  impl_->manager->set_dispatcher(func);
}
  
const plain::net::packet::dispatch_func &Connector::dispatcher() const noexcept {
  return impl_->manager->dispatcher();
}
   
void Connector::set_connect_callback(connection::callable_func func) noexcept {
  impl_->manager->set_connect_callback(func);
}
  
void Connector::set_disconnect_callback(
  connection::callable_func func) noexcept {
  impl_->manager->set_disconnect_callback(func);
}
 
std::shared_ptr<plain::net::connection::Basic>
Connector::get_conn(connection::id_t id) const noexcept {
  return impl_->manager->get_conn(id);
}
  
bool Connector::is_full() const noexcept {
  return impl_->manager->is_full();
}
  
void Connector::broadcast(std::shared_ptr<packet::Basic> packet) noexcept {
  return impl_->manager->broadcast(packet);
}
  
std::shared_ptr<plain::concurrency::executor::Basic>
Connector::get_executor() const noexcept {
  return impl_->manager->get_executor();
}

std::shared_ptr<plain::net::connection::Basic>
Connector::connect_impl(
  std::string_view addr_or_ip, uint16_t port,
  std::function<bool(connection::Basic *)> init_func,
  const std::chrono::milliseconds &timeout,
  socket::Type sock_type) noexcept {
  auto conn = impl_->manager->new_conn();
  if (!conn || conn->id() == connection::kInvalidId) return {};
  if (static_cast<bool>(init_func) && !init_func(conn.get())) {
    impl_->manager->remove(conn, true, false);
    LOG_ERROR << "initialize failed";
    return {};
  }
  auto success = port == 0 ?
    conn->socket()->connect(addr_or_ip, timeout, sock_type) :
    conn->socket()->connect(addr_or_ip, port, timeout, sock_type);
  if (!success) {
    impl_->manager->remove(conn, true, false);
    std::string addr{addr_or_ip};
    if (port != 0) addr += ":" + std::to_string(port);
    LOG_ERROR << "connect " << addr << " failed: " << socket::get_last_error();
    return {};
  }
  auto sock = conn->socket();
  if (!sock->set_nonblocking()) {
    impl_->manager->remove(conn, true, false);
    LOG_ERROR << "set_nonblocking failed: " << socket::get_last_error();
    return {};
  }
  if (!sock->set_linger(0)) {
    impl_->manager->remove(conn, true, false);
    LOG_ERROR << "set_linger(0) failed: " << socket::get_last_error();
    return {};
  }
  if (!impl_->manager->sock_add(sock->id(), conn->id())) {
    impl_->manager->remove(conn, true, false);
    LOG_ERROR << "sock add failed";
    return {};
  }
  impl_->manager->send_ctrl_cmd("w"); // wakeup to add socket descriptor.
  conn->on_connect();
  const auto &callback = impl_->manager->connect_callback();
  if (static_cast<bool>(callback))
    callback(conn.get());
  return conn;
}
  
void Connector::set_keep_alive(
	std::shared_ptr<connection::Basic> conn, bool flag) noexcept {
   auto _conn = get_conn(conn->id());
   if (!_conn || _conn != conn) return;
   conn->set_keep_alive(flag);
}
  
bool Connector::is_keep_alive(
    std::shared_ptr<connection::Basic> conn) const noexcept {
  return static_cast<bool>(conn) && conn->is_keep_alive();
}
 
void Connector::set_keep_alive(connection::Basic *conn, bool flag) noexcept {
  if (!conn) return;
  auto _conn = get_conn(conn->id());
  if (!_conn || _conn.get() != conn) return;
  conn->set_keep_alive(flag);
}
  
bool Connector::is_keep_alive(connection::Basic *conn) const noexcept {
  return !is_null(conn) && conn->is_keep_alive();
} 

bool Connector::connect(
  std::shared_ptr<connection::Basic> conn,
  const std::chrono::milliseconds &timeout) noexcept {
  return connect(conn.get(), timeout);
}

bool Connector::connect(
  connection::Basic *conn,
  const std::chrono::milliseconds &timeout) noexcept {
  if (!conn || conn->id() == connection::kInvalidId) return false;
  auto addr = conn->socket()->peer_address().text();
  if (addr.empty()) return false;
  auto success = conn->socket()->connect(addr, timeout, conn->socket()->type());
  if (!success) {
    LOG_ERROR << "connect " << addr << " failed: " << socket::get_last_error();
    return false;
  }
  auto sock = conn->socket();
  if (!sock->set_nonblocking()) {
    LOG_ERROR << "set_nonblocking failed: " << socket::get_last_error();
    return false;
  }
  if (!sock->set_linger(0)) {
    LOG_ERROR << "set_linger(0) failed: " << socket::get_last_error();
    return false;
  }
  if (!impl_->manager->sock_add(sock->id(), conn->id())) {
    LOG_ERROR << "sock add failed";
    return false;
  }
  impl_->manager->send_ctrl_cmd("w"); // wakeup to add socket descriptor.
  conn->on_connect();
  const auto &callback = impl_->manager->connect_callback();
  if (static_cast<bool>(callback))
    callback(conn);
  return true;
}

uint64_t Connector::send_size() const noexcept {
  return impl_->manager->send_size();
}

uint64_t Connector::recv_size() const noexcept {
  return impl_->manager->recv_size();
}
  
bool Connector::running() const noexcept {
  return impl_->manager->running();
}
