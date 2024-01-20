#include "plain/net/connection/manager.h"
#include <set>
#include "plain/concurrency/executor/basic.h"
#include "plain/concurrency/executor/thread.h"
#include "plain/net/connection/detail/coroutine.h"
#include "plain/net/connection/basic.h"
#include "plain/net/socket/api.h"
#include "plain/net/socket/basic.h"
#include "plain/net/socket/listener.h"

using plain::net::connection::Manager;
using namespace std::chrono_literals;

namespace plain::net::connection::detail {
namespace {

struct ConnectionInfo {
  std::vector<std::shared_ptr<plain::net::connection::Basic>> list;
  std::set<id_t> free_ids;
  size_t size{0};
  id_t max_id{0};
};

}
}

struct Manager::Impl {
  stream::codec_t codec;
  packet::dispatch_func dispatcher;
  std::unique_ptr<concurrency::executor::Basic> executor;
  detail::ConnectionInfo connection_info;
  std::mutex mutex;
  std::mutex wait_mutex;
  std::condition_variable cv;
  thread_t worker; // The main worker.
  detail::task_handle_t task_handle;
  std::atomic_bool tasking{false};

  void init_connections(uint32_t count);
  static plain::net::connection::detail::Task
  work(std::shared_ptr<Manager> manager) noexcept;
  static void enqueue_work(std::shared_ptr<Manager> manager) noexcept;
  static bool wait_work(std::shared_ptr<Manager> manager) noexcept;
  static void handle(std::shared_ptr<Manager> manager) noexcept;

};

void Manager::Impl::init_connections(uint32_t count) {
  connection_info.list.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    auto ptr = std::make_shared<Basic>();
    ptr->init();
    connection_info.list.emplace_back(ptr);
    //connection_info.list[i] = ptr;
    //connection_info.list[i]->init();
  }
}

void Manager::Impl::enqueue_work(std::shared_ptr<Manager> manager) noexcept {
  assert(manager);
  if (!manager->running_ || manager->impl_->tasking) return; // The work in executor wait resume.
  manager->get_executor().post([manager]() {
    work(manager);
  });
}

bool Manager::Impl::wait_work(std::shared_ptr<Manager> manager) noexcept {
  assert(manager);
  std::unique_lock<std::mutex> lock{manager->impl_->wait_mutex};
  if (manager->impl_->connection_info.size > 0) {
    /*
    auto old_size = manager->impl_->connection_info.size;
    manager->impl_->cv.wait_for(lock, 10ms, [manager, old_size] {
      return old_size != manager->impl_->connection_info.size ||
        !manager->running_;
    });
    */
  } else {
    manager->impl_->cv.wait(lock, [manager] {
      return manager->impl_->connection_info.size > 0 || !manager->running_;
    });
  }
  manager->impl_->cv.wait(lock, [manager] {
    return manager->impl_->tasking || !manager->running_;
  });
  if (manager->impl_->tasking) {
    manager->impl_->cv.wait(lock, [manager] {
      return static_cast<bool>(manager->impl_->task_handle); // || !manager->running_;
    });
  }
  manager->impl_->handle(manager);
  manager->impl_->cv.wait(lock, [manager] {
    const auto tasking = manager->impl_->tasking.load(std::memory_order_relaxed);
    return !tasking; // || !manager->running_;
  });
  //const auto tasking = manager->impl_->tasking.load(std::memory_order_relaxed);
  //std::cout << "tasking:" << (tasking ? 1: 0) << ":" << manager << std::endl;
  if (!manager->running_) return false;
  enqueue_work(manager);
  return true;
}

plain::net::connection::detail::Task
Manager::Impl::work(std::shared_ptr<Manager> manager) noexcept {
  auto func = [manager](detail::task_handle_t handle) {
    assert(!static_cast<bool>(manager->impl_->task_handle));
    assert(handle);
    if (!manager->running_) {
      handle();
    } else {
      manager->impl_->task_handle = std::move(handle);
      manager->impl_->cv.notify_one();
    }
    return true;
  };
  //std::cout << "waiting...: " << manager << std::endl;
  manager->impl_->tasking.store(true, std::memory_order_relaxed);
  co_await detail::Awaitable{func};
  //std::cout << "working...: " << manager << std::endl;
  auto r = manager->work();
  if (!r) manager->stop();
  manager->impl_->tasking.store(false, std::memory_order_relaxed);
  manager->impl_->cv.notify_one();
  //std::cout << "work end: " << manager << std::endl;
}

void Manager::Impl::handle(std::shared_ptr<Manager> manager) noexcept {
  if (static_cast<bool>(manager->impl_->task_handle)) {
    manager->impl_->task_handle();
    manager->impl_->task_handle = detail::task_handle_t{};
  }
}

Manager::Manager(const setting_t &setting) : 
  setting_{setting}, impl_{std::make_unique<Impl>()} {
  assert(setting.default_count <= setting.max_count);
  impl_->executor = std::move(std::make_unique<concurrency::executor::Thread>());
  impl_->init_connections(setting.default_count);
}

Manager::Manager(
  std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) :
  setting_{setting}, impl_{std::make_unique<Impl>()} {
  assert(setting.default_count <= setting.max_count);
  impl_->executor = std::move(executor);
  impl_->init_connections(setting.default_count);
}

Manager::~Manager() = default;

bool Manager::start() {
  if (!prepare()) return false;
  running_ = true;
  Impl::enqueue_work(shared_from_this());
  impl_->worker = std::move(thread_t([manager = shared_from_this()]{
    for (;;) {
      if (!Impl::wait_work(manager)) break;
    }
    //std::cout << "work exit: " << manager << std::endl;
  }));
  return true;
}

void Manager::stop() {
  running_ = false;
  off();
  impl_->cv.notify_one(); // Just one for wait work.
  impl_->executor->shutdown();
  std::this_thread::sleep_for(10ms);
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
  std::unique_lock<decltype(impl_->mutex)> auto_lock(impl_->mutex);
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
    }
  }
  if (id == kInvalidId) return {};
  auto r = impl_->connection_info.list[id - 1];
  if (!r) r = std::move(std::make_shared<Basic>());
  r->set_id(id);
  r->init();
  r->set_manager(shared_from_this());
  ++impl_->connection_info.size;
  impl_->cv.notify_one(); // Just one for wait work.
  return r;
}
  
bool Manager::is_full() const noexcept {
  return impl_->connection_info.free_ids.empty() &&
    impl_->connection_info.max_id != kInvalidId &&
    impl_->connection_info.max_id >= static_cast<id_t>(setting_.max_count);
}

void Manager::remove(std::shared_ptr<Basic> conn) noexcept {
  if (conn->id() == connection::kInvalidId) return;
  remove(conn->id());
}
  
void Manager::remove(connection::id_t conn_id) noexcept {
  std::unique_lock<decltype(impl_->mutex)> auto_lock(impl_->mutex);
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
  --impl_->connection_info.size;
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

std::shared_ptr<plain::net::connection::Basic> Manager::accept() noexcept {
  if (listen_fd_ == socket::kInvalidSocket || !listen_sock_)
    return {};
  auto conn = new_conn();
  if (!conn) {
    LOG_WARN << "new conn failed";
    auto sock = std::make_shared<socket::Basic>();
    listen_sock_->accept(sock);
    sock->close();
    return {};
  }
  if (!listen_sock_->accept(conn->socket())) {
    remove(conn);
    LOG_ERROR << "connection accept error: " << socket::get_last_error();
    return {};
  }
  if (!conn->socket()->set_nonblocking()) {
    remove(conn);
    LOG_ERROR << "set_nonblocking error: " << socket::get_last_error();
    return {};
  }
  if (conn->socket()->error()) {
    remove(conn);
    LOG_ERROR << "socket have error: " << socket::get_last_error();
    return {};
  }
  if (!conn->socket()->set_linger(0)) {
    remove(conn);
    LOG_ERROR << "set_linger error: " << socket::get_last_error();
    return {};
  }
  if (!sock_add(conn->socket()->id(), conn->id())) {
    remove(conn);
    LOG_ERROR << "sock_add error";
    return {};
  }
  conn->on_connect();
  return conn;
}
