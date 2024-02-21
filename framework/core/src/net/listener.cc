#include "plain/net/listener.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/connection/manager.h"
#include "plain/net/socket/listener.h"
#include "plain/net/utility.h"

using plain::net::Listener;

struct Listener::Impl {
  std::shared_ptr<connection::Manager> manager;
};

Listener::Listener(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) : impl_{std::make_unique<Impl>()} {
  impl_->manager = make_manager(
    std::forward<decltype(executor)>(executor), setting);
  assert(impl_->manager);
}
  
Listener::Listener(const setting_t &setting) : impl_{std::make_unique<Impl>()} {
  impl_->manager = make_manager(setting);
  assert(impl_->manager);
}

Listener::~Listener() {
  if (impl_->manager->running()) impl_->manager->stop();
}
  
bool Listener::start() {
  if (impl_->manager->running()) return true;
  impl_->manager->listen_sock_ = std::make_shared<socket::Listener>();
  Address addr{impl_->manager->setting_.address};
  if (!impl_->manager->listen_sock_->init(addr)) return false;
  impl_->manager->listen_fd_ = impl_->manager->listen_sock_->id();
  return impl_->manager->start();
}
  
void Listener::stop() {
  // Manager must stoped.
  if (impl_->manager->running()) impl_->manager->stop();
}

void Listener::set_codec(const stream::codec_t &codec) noexcept {
  impl_->manager->set_codec(codec);
}
  
const plain::net::stream::codec_t &Listener::codec() const noexcept {
  return impl_->manager->codec();
}
  
void Listener::set_dispatcher(packet::dispatch_func func) noexcept {
  impl_->manager->set_dispatcher(func);
}
  
const plain::net::packet::dispatch_func &Listener::dispatcher() const noexcept {
  return impl_->manager->dispatcher();
}
  
void Listener::set_connect_callback(connection::callable_func func) noexcept {
  impl_->manager->set_connect_callback(func);
}
  
void Listener::set_disconnect_callback(
  connection::callable_func func) noexcept {
  impl_->manager->set_disconnect_callback(func);
}

std::shared_ptr<plain::net::connection::Basic>
Listener::get_conn(connection::id_t id) const noexcept {
  return impl_->manager->get_conn(id);
}
  
bool Listener::is_full() const noexcept {
  return impl_->manager->is_full();
}
  
void Listener::broadcast(std::shared_ptr<packet::Basic> packet) noexcept {
  return impl_->manager->broadcast(packet);
}
  
plain::concurrency::executor::Basic &Listener::get_executor() {
  return impl_->manager->get_executor();
}
  
plain::net::Address Listener::address() const noexcept {
  if (!impl_->manager->listen_sock_) return {};
  return impl_->manager->listen_sock_->address();
}
