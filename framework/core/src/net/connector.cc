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
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) : impl_{std::make_unique<Impl>()} {
  impl_->manager = make_manager(
    setting.mode, std::forward<decltype(executor)>(executor), setting);
  assert(impl_->manager);
}
  
Connector::Connector(const setting_t &setting) :
  impl_{std::make_unique<Impl>()} {
  impl_->manager = make_manager(setting.mode, setting);
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
  std::string_view address, const std::chrono::milliseconds &timeout) noexcept {
  return connect_impl(address, 0, timeout);
}
 
std::shared_ptr<plain::net::connection::Basic>
Connector::connect(
  std::string_view ip, uint16_t port,
  const std::chrono::milliseconds &timeout) noexcept {
  return connect_impl(ip, port, timeout);
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
Connector::get_conn(id_t id) const noexcept {
  return impl_->manager->get_conn(id);
}
  
bool Connector::is_full() const noexcept {
  return impl_->manager->is_full();
}
  
void Connector::broadcast(std::shared_ptr<packet::Basic> packet) noexcept {
  return impl_->manager->broadcast(packet);
}
  
plain::concurrency::executor::Basic &Connector::get_executor() {
  return impl_->manager->get_executor();
}

std::shared_ptr<plain::net::connection::Basic>
Connector::connect_impl(
  std::string_view addr_or_ip, uint16_t port,
  const std::chrono::milliseconds &timeout) noexcept {
  auto conn = impl_->manager->new_conn();
  if (!conn || conn->id() == connection::kInvalidId) return {};
  auto success = port == 0 ?
    conn->socket()->connect(addr_or_ip, timeout) :
    conn->socket()->connect(addr_or_ip, port, timeout);
  if (!success) {
    impl_->manager->remove(conn, true);
    LOG_ERROR << "connect failed: " << socket::get_last_error();
    return {};
  }
  auto sock = conn->socket();
  if (!sock->set_nonblocking()) {
    impl_->manager->remove(conn, true);
    LOG_ERROR << "set_nonblocking failed: " << socket::get_last_error();
    return {};
  }
  if (!sock->set_linger(0)) {
    impl_->manager->remove(conn, true);
    LOG_ERROR << "set_linger(0) failed: " << socket::get_last_error();
    return {};
  }
  if (!impl_->manager->sock_add(sock->id(), conn->id())) {
    impl_->manager->remove(conn, true);
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
