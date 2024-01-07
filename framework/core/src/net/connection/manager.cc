#include "plain/net/connection/manager.h"
#include <set>
#include "plain/concurrency/executor/basic.h"
#include "plain/concurrency/executor/thread.h"
#include "plain/net/socket/basic.h"
#include "plain/net/connection/basic.h"

using plain::net::connection::Manager;

namespace plain::net::connection::detail {
namespace {

struct ConnectionInfo {
  std::vector<std::shared_ptr<plain::net::connection::Basic>> list;
  std::set<id_t> free_ids;
  size_t size{0};
  id_t max_id{static_cast<id_t>(plain::net::connection::kInvalidId)};
};

}
}

struct Manager::Impl {
  stream::codec_t codec;
  packet::dispatch_func dispatcher;
  std::unique_ptr<concurrency::executor::Basic> executor;
  detail::ConnectionInfo connection_info;
  void init_connections(uint32_t count, std::shared_ptr<Manager> manager);
};

void Manager::Impl::init_connections(
  uint32_t count, std::shared_ptr<Manager> manager) {
  connection_info.list.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    connection_info.list[i] = std::move(std::make_shared<Basic>());
    connection_info.list[i]->init();
    connection_info.list[i]->set_manager(manager);
  }
}

Manager::Manager(const setting_t &setting) : 
  setting_{setting}, impl_{std::make_unique<Impl>()} {
  impl_->executor = std::move(std::make_unique<concurrency::executor::Thread>());
  impl_->init_connections(setting.default_count, shared_from_this());
}

Manager::Manager(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  setting_{setting}, impl_{std::make_unique<Impl>()} {
  impl_->executor = std::move(executor);
  impl_->init_connections(setting.default_count, shared_from_this());
}

Manager::~Manager() = default;

bool Manager::start() {
  return work();
}

void Manager::stop() {
  off();
}
  
plain::concurrency::executor::Basic &Manager::get_executor() {
  return *impl_->executor.get();
}

void Manager::set_codec(const stream::codec_t &codec) noexcept {
  impl_->codec = codec;
}
  
const plain::net::stream::codec_t &Manager::codec() const noexcept {
  return impl_->codec;
}

void Manager::set_packet_dispatcher(packet::dispatch_func func) noexcept {
  impl_->dispatcher = func;
}
  
const plain::net::packet::dispatch_func &Manager::dispatcher() const noexcept {
  return impl_->dispatcher;
}

std::shared_ptr<plain::net::connection::Basic>
Manager::get_conn(id_t id) const noexcept {
  auto index = static_cast<size_t>(id) - 1;
  if (index >= impl_->connection_info.list.size())
    return {};
  auto r = impl_->connection_info.list[index];
  if (!r || !r->valid()) return {};
  return r;
}

std::shared_ptr<plain::net::connection::Basic> Manager::new_conn() noexcept {
  if (is_full()) {
    return {};
  }
  id_t id{kInvalidId};
  if (!impl_->connection_info.free_ids.empty()) {
    auto it = impl_->connection_info.free_ids.begin();
    id = *it;
    impl_->connection_info.free_ids.erase(it);
  } else {
    id = ++(impl_->connection_info.max_id);
    if (static_cast<size_t>(id) > impl_->connection_info.list.size()) {
      impl_->connection_info.list.emplace_back(std::make_shared<Basic>());
      (*impl_->connection_info.list.rbegin())->set_manager(shared_from_this());
    }
  }
  if (id == kInvalidId) return {};
  auto &r = impl_->connection_info.list[id - 1];
  if (!r) r = std::move(std::make_shared<Basic>());
  r->init();
  return r;
}
  
bool Manager::is_full() const noexcept {
  return impl_->connection_info.free_ids.empty() &&
    impl_->connection_info.max_id != kInvalidId &&
    impl_->connection_info.max_id >= static_cast<id_t>(setting_.max_count);
}

void Manager::remove(std::shared_ptr<Basic> conn) noexcept {
  remove(conn->id());
}
  
void Manager::remove(connection::id_t conn_id) noexcept {
  assert(conn_id != connection::kInvalidId);
  if (conn_id <= 0 || conn_id > impl_->connection_info.max_id)
    return;
  auto &conn = impl_->connection_info.list[conn_id - 1];
  if (conn) {
    if (conn->valid()) conn->on_disconnect();
    if (conn->socket()->valid()) this->sock_remove(conn->socket()->id());
    conn->close();
  }
  impl_->connection_info.free_ids.emplace(conn_id);
}
  
void Manager::broadcast(std::shared_ptr<packet::Basic> packet) noexcept {
  assert(packet);
  for (auto &conn : impl_->connection_info.list) {
    if (conn->valid()) conn->send(packet);
  }
}

void Manager::foreach(std::function<void(std::shared_ptr<Basic> conn)> func) {
  for (auto conn : impl_->connection_info.list) {
    if (conn && conn->valid()) func(conn);
  }
}
