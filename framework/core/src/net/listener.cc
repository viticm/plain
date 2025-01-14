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
  const setting_t &setting,
  std::shared_ptr<concurrency::executor::Basic> executor) :
  rpc_dispatcher_{std::make_shared<rpc::Dispatcher>()},
  impl_{std::make_unique<Impl>()} {
  impl_->manager = make_manager(setting, executor);
  impl_->manager->set_rpc_dispatcher(rpc_dispatcher_);
  assert(impl_->manager);
}

Listener::~Listener() {
  if (impl_->manager->running()) impl_->manager->stop();
}
  
bool Listener::start() {
  if (impl_->manager->running()) return true;
  impl_->manager->listen_sock_ = std::make_shared<socket::Listener>();
  Address addr{impl_->manager->setting_.address};
  if (!impl_->manager->listen_sock_->init(
      addr, impl_->manager->setting_.socket_type))
    return false;
  if (impl_->manager->setting_.address.empty())
    impl_->manager->setting_.address = address().text();
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
  
std::shared_ptr<plain::concurrency::executor::Basic>
Listener::get_executor() const noexcept {
  return impl_->manager->get_executor();
}
  
plain::net::Address Listener::address() const noexcept {
  if (!impl_->manager->listen_sock_) return {};
  return impl_->manager->listen_sock_->address();
}

uint64_t Listener::send_size() const noexcept {
  return impl_->manager->send_size();
}

uint64_t Listener::recv_size() const noexcept {
  return impl_->manager->recv_size();
}

bool Listener::running() const noexcept {
  return impl_->manager->running();
}
