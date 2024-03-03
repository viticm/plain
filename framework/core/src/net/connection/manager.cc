#include "plain/net/connection/manager.h"
#include <set>
#include <array>
#include <deque>
#include "plain/basic/utility.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/concurrency/executor/worker_thread.h"
#include "plain/net/detail/coroutine.h"
#include "plain/net/connection/basic.h"
#include "plain/net/socket/api.h"
#include "plain/net/socket/basic.h"
#include "plain/net/socket/listener.h"

// Current coroutine implemention not recommand.
// #define PLAIN_NET_MANAGER_ENABLE_COROUTINE

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
  std::shared_ptr<concurrency::executor::Basic> executor;
  detail::ConnectionInfo connection_info;
  std::mutex mutex;
#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
  std::mutex wait_mutex;
  std::condition_variable cv;
  thread_t worker; // The main worker.
#endif
  socket::id_t ctrl_write_fd{socket::kInvalidId};
  bool running{false};
  callable_func connect_callback;
  callable_func disconnect_callback;
  uint64_t send_size{0};
  uint64_t recv_size{0};
  // This values for enqueue connection works.
  std::atomic_uint32_t working_conn_count{0};
  std::deque<connection::id_t> wait_work_conn_id_deque;
  std::set<connection::id_t> wait_work_conn_ids;
  int32_t working_conn_max_count{32};

  void init_connections(uint32_t count);
#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
  static bool wait_work(std::shared_ptr<Manager> manager) noexcept;
#else
  static void enqueue_work_await(std::shared_ptr<Manager> manager) noexcept;
  static plain::net::detail::Task<bool>
  work_await(std::shared_ptr<Manager> manager) noexcept;
#endif
  static void work(
    std::shared_ptr<Manager> manager, connection::id_t conn_id) noexcept;

};

void Manager::Impl::init_connections(uint32_t count) {
  connection_info.list.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    auto ptr = std::make_shared<Basic>();
    ptr->init();
    connection_info.list.emplace_back(ptr);
    /*
    connection_info.list[i] = ptr;
    connection_info.list[i]->init();
    */
  }
}

#ifdef PLAIN_NET_MANAGER_ENABLE_COROUTINE

void Manager::Impl::enqueue_work_await(
  std::shared_ptr<Manager> manager) noexcept {
  // std::cout << "enqueue_work_await: " << manager << std::endl;
  assert(manager);
  if (!manager->running()) return;
  // The work in executor wait resume.
  manager->execute([manager] {
    if (!manager->running()) return;
    auto executor = manager->get_executor();
    if (!static_cast<bool>(executor)) return;
    if (executor->shutdown_requested()) return;
    std::weak_ptr<Manager> m{manager};
    executor->post([m]() -> plain::net::detail::Task<> {
      auto manager = m.lock();
      if (manager) {
        auto r = co_await manager->impl_->work_await(manager);
        // std::cout << "work: " << manager << "|" << r << std::endl;
        if (r)
          manager->impl_->enqueue_work_await(manager);
      }
    });
  });
}

plain::net::detail::Task<bool>
Manager::Impl::work_await(std::shared_ptr<Manager> manager) noexcept {
  auto resolve = [manager](void *d) {
    thread_t([manager, d] {
      auto resolver = static_cast<net::detail::ResumeResolver *>(d);
      if (!resolver) return;
      auto r = manager->work();
      resolver->resolve(r ? 1 : 0);
    }).detach();
  };
  auto r = co_await net::detail::Awaitable{resolve};
  co_return r == 1;
  // std::cout << "work_await: " << manager << "|" << r << "|" <<
  // (int32_t)manager->setting_.mode << std::endl;
}
#endif

#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
bool Manager::Impl::wait_work(std::shared_ptr<Manager> manager) noexcept {
  assert(manager);
  std::unique_lock<std::mutex> lock{manager->impl_->wait_mutex};
  manager->impl_->cv.wait(lock, [manager] {
    return manager->listen_fd_ != socket::kInvalidId ||
      manager->impl_->connection_info.size > 0 || !manager->running();
  });
  if (!manager->running() || !manager->work()) return false;
  // std::this_thread::sleep_for(1ms); // change to executor thread work.
  return true;
}
#endif

void Manager::Impl::work(
  std::shared_ptr<Manager> manager, connection::id_t id) noexcept {
  auto conn = manager->get_conn(id);
  bool r{false};
  {
    scoped_executor_t scoped_executor([manager]() {
        manager->impl_->working_conn_count.store(
        manager->impl_->working_conn_count.load(std::memory_order_relaxed) - 1,
        std::memory_order_relaxed);
    });
    if (!conn) return;
    if (!conn->valid()) return;
    r = conn->work();
  }
  if (!r) {
    manager->remove(conn);
  } else {
    if (!conn->idle()) {
      manager->impl_->executor->enqueue([manager, id]{
        manager->enqueue(id);
      });
    }
  }
}

Manager::Manager(
  const setting_t &setting,
  std::shared_ptr<concurrency::executor::Basic> executor) :
  setting_{setting}, impl_{std::make_unique<Impl>()} {
  assert(setting.default_count <= setting.max_count);
  if (!executor) {
    executor = std::make_shared<concurrency::executor::WorkerThread>();
  }
  impl_->executor = executor;
  impl_->init_connections(setting.default_count);
  impl_->working_conn_max_count = impl_->executor->max_concurrency_level();
  if (impl_->working_conn_max_count > 0) impl_->working_conn_max_count *= 2;
}

Manager::~Manager() {
  if (ctrl_read_fd_ != socket::kInvalidId)
    socket::close(ctrl_read_fd_);
  if (impl_->ctrl_write_fd != socket::kInvalidId)
    socket::close(impl_->ctrl_write_fd);
}

bool Manager::start() {
  if (impl_->running) return true;
  if (!socket::initialize()) return false;
  if (!prepare()) return false;
  socket::id_t fds[2]{socket::kInvalidId};
  if (socket::make_pair(fds)) {
    ctrl_read_fd_ = fds[0];
    impl_->ctrl_write_fd = fds[1];
    // std::cout << "fds: " << fds[0] << "|" << fds[1] << std::endl;
    if (!sock_add(ctrl_read_fd_, 0)) {
      LOG_ERROR << setting_.name << " add ctrl read fd failed";
      return false;
    }
  }
  impl_->running = true;
#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
   impl_->worker = thread_t([manager = shared_from_this()]{
    for (;;) {
      if (!Impl::wait_work(manager)) break;
    }
    // std::cout << "work exit: " << manager << std::endl;
  });
#else
  impl_->enqueue_work_await(shared_from_this());
#endif
  return true;
}

void Manager::stop() {
  // std::cout << "stop: " << this << std::endl;
  {
    std::unique_lock<std::mutex> lock{impl_->mutex};
    if (!impl_->running) return;
    impl_->running = false;
  }
  off();
  for (auto conn : impl_->connection_info.list) {
    if (conn && conn->valid()) conn->shutdown();
  }
#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
  impl_->cv.notify_one(); // Just one for wait work.
#endif
  send_ctrl_cmd("k"); // Send stop.

#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
  // The will get "Resource deadlock avoided" except if not join(destroy join).
  // Worker stop must be before the executor(connection work in executor and 
  // if open coroutine mode manager wait work also).
  if (impl_->worker.joinable())
    impl_->worker.join();
#endif
  std::this_thread::sleep_for(1ms);
  impl_->executor->shutdown();
}

bool Manager::running() const noexcept {
  return impl_->running;
}
  
std::shared_ptr<plain::concurrency::executor::Basic>
Manager::get_executor() const noexcept {
  return impl_->executor;
}

void Manager::set_codec(const stream::codec_t &codec) noexcept {
  impl_->codec = codec;
}
  
const plain::net::stream::codec_t &Manager::codec() const noexcept {
  return impl_->codec;
}

void Manager::set_dispatcher(packet::dispatch_func func) noexcept {
  impl_->dispatcher = func;
}
  
const plain::net::packet::dispatch_func &Manager::dispatcher() const noexcept {
  return impl_->dispatcher;
}

void Manager::set_connect_callback(callable_func func) noexcept {
  impl_->connect_callback = func;
}

const plain::net::connection::callable_func &
Manager::connect_callback() const noexcept {
  return impl_->connect_callback;
}
 
void Manager::set_disconnect_callback(callable_func func) noexcept {
  impl_->disconnect_callback = func;
}
  
std::shared_ptr<plain::net::connection::Basic>
Manager::get_conn(id_t id) const noexcept {
  auto index = static_cast<size_t>(id) - 1;
  if (index >= impl_->connection_info.list.size())
    return {};
  auto r = impl_->connection_info.list[index];
  // if (!r || !r->valid()) return {};
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
  if (!r) r = std::make_shared<Basic>();
  r->set_id(id);
  r->init();
  r->set_manager(shared_from_this());
  ++(impl_->connection_info.size);

#ifndef PLAIN_NET_MANAGER_ENABLE_COROUTINE
  impl_->cv.notify_one(); // Just one for wait work.
#endif
  return r;
}
  
bool Manager::is_full() const noexcept {
  return impl_->connection_info.free_ids.empty() &&
    impl_->connection_info.max_id != kInvalidId &&
    impl_->connection_info.max_id >= static_cast<id_t>(setting_.max_count);
}

void Manager::remove(std::shared_ptr<Basic> conn, bool no_event) noexcept {
  if (conn->id() == connection::kInvalidId) return;
  remove(conn->id(), no_event);
}
  
void Manager::remove(connection::id_t conn_id, bool no_event) noexcept {
  std::unique_lock<decltype(impl_->mutex)> auto_lock(impl_->mutex);
  assert(conn_id != connection::kInvalidId);
  if (conn_id <= 0 || conn_id > impl_->connection_info.max_id)
    return;
  auto &conn = impl_->connection_info.list[conn_id - 1];
  // std::cout << "remove: " << conn_id << std::endl;
  if (conn) {
    if (!no_event) {
      if (static_cast<bool>(impl_->disconnect_callback))
        impl_->disconnect_callback(conn.get());
      conn->on_disconnect();
    }
    if (conn->socket()->valid()) this->sock_remove(conn->socket()->id());
    conn->shutdown();
    conn->close();
  }
  impl_->connection_info.free_ids.emplace(conn_id);
  if (impl_->connection_info.size > 0)
    --(impl_->connection_info.size);
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
  if (listen_fd_ == socket::kInvalidId || !listen_sock_)
    return {};
  auto conn = new_conn();
  if (!conn) {
    LOG_WARN << setting_.name << " new conn failed";
    auto sock = std::make_shared<socket::Basic>();
    listen_sock_->accept(sock);
    sock->close();
    return {};
  }
  if (!listen_sock_->accept(conn->socket())) {
    remove(conn, true);
    auto last_error = socket::get_last_error();
    if (last_error.code() != socket::kErrorWouldBlock) {
      LOG_ERROR << setting_.name <<
        " connection accept error: " << last_error;
    }
    return {};
  }
  if (!conn->socket()->set_nonblocking()) {
    remove(conn, true);
    LOG_ERROR << setting_.name << " set_nonblocking error: " <<
      socket::get_last_error();
    return {};
  }
  if (conn->socket()->error()) {
    remove(conn, true);
    LOG_ERROR << setting_.name << " socket have error: " <<
      socket::get_last_error();
    return {};
  }
  if (!conn->socket()->set_linger(0)) {
    remove(conn, true);
    LOG_ERROR << setting_.name << "set_linger error: " <<
      socket::get_last_error();
    return {};
  }
  if (!sock_add(conn->socket()->id(), conn->id())) {
    remove(conn, true);
    LOG_ERROR << setting_.name << " sock_add error";
    return {};
  }
  conn->on_connect();
  if (static_cast<bool>(impl_->connect_callback)) {
    impl_->connect_callback(conn.get());
  }
  return conn;
}
  
std::shared_ptr<plain::net::connection::Basic>
Manager::accept(socket::id_t sock_id) noexcept {
  auto conn = new_conn();
  if (!conn) {
    LOG_WARN << setting_.name << " new conn failed";
    auto sock = std::make_shared<socket::Basic>();
    listen_sock_->accept(sock);
    sock->close();
    return {};
  }
  if (sock_id == socket::kInvalidId) return {};
  conn->socket()->set_id(sock_id);
  if (!conn->socket()->set_nonblocking()) {
    remove(conn, true);
    LOG_ERROR << setting_.name << " set_nonblocking error: " <<
      socket::get_last_error();
    return {};
  }
  if (conn->socket()->error()) {
    remove(conn, true);
    LOG_ERROR << setting_.name << " socket have error: " <<
      socket::get_last_error();
    return {};
  }
  if (!conn->socket()->set_linger(0)) {
    remove(conn, true);
    LOG_ERROR << setting_.name << "set_linger error: " <<
      socket::get_last_error();
    return {};
  }
  if (!sock_add(conn->socket()->id(), conn->id())) {
    remove(conn, true);
    LOG_ERROR << setting_.name << " sock_add error";
    return {};
  }
  conn->on_connect();
  if (static_cast<bool>(impl_->connect_callback)) {
    impl_->connect_callback(conn.get());
  }
  return conn;
}

bool Manager::send_ctrl_cmd(std::string_view cmd) noexcept {
  if (impl_->ctrl_write_fd == socket::kInvalidId)
    return false;
  auto size = static_cast<uint32_t>(cmd.size());
  auto r = socket::send(impl_->ctrl_write_fd, cmd.data(), size, 0);
  return r >= 0;
}
  
void Manager::recv_ctrl_cmd() noexcept {
  if (ctrl_read_fd_ == socket::kInvalidId)
    return;
  std::array<char, 256> buffer{0};
  auto size = static_cast<uint32_t>(buffer.size());
  socket::recv(ctrl_read_fd_, buffer.data(), size, 0);
  auto type = buffer[0];
  switch (type) {
    case 'w': // wakeup
    case 'k': // stop
      break;
    default:
      LOG_ERROR << setting_.name << " unknown cmd: " << buffer.data();
      break;
  }
  // std::cout << "recv_ctrl_cmd: " << buffer.data() << std::endl;
}

void Manager::execute(std::function<void()> func) {
  // This use with get_executor post functions(How can free?).
  std::unique_lock<std::mutex> lock{impl_->mutex};
  func();
}

void Manager::increase_send_size(size_t size) {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  impl_->send_size += static_cast<uint64_t>(size);
}
  
void Manager::increase_recv_size(size_t size) {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  impl_->recv_size += static_cast<uint64_t>(size);
}

uint64_t Manager::send_size() const noexcept {
  return impl_->send_size;
}

uint64_t Manager::recv_size() const noexcept {
  return impl_->recv_size;
}

// For banlance connection works.
void Manager::enqueue(connection::id_t id) noexcept {
  auto working_conn_count =
    impl_->working_conn_count.load(std::memory_order_relaxed);
  if (impl_->working_conn_max_count <= 0 ||
      working_conn_count < static_cast<uint32_t>(
        impl_->working_conn_max_count)) {
    if (id == connection::kInvalidId) {
      std::unique_lock<std::mutex> lock{impl_->mutex};
      if (impl_->wait_work_conn_id_deque.empty()) return;
      id = impl_->wait_work_conn_id_deque.front();
      impl_->wait_work_conn_id_deque.pop_front();
      auto it = impl_->wait_work_conn_ids.find(id);
      if (it != impl_->wait_work_conn_ids.end())
        impl_->wait_work_conn_ids.erase(it);
    }
    if (id == connection::kInvalidId) return;
    impl_->working_conn_count.store(
      impl_->working_conn_count.load(std::memory_order_relaxed) + 1,
      std::memory_order_relaxed);
    std::cout << "id: " << id << std::endl;
    impl_->executor->enqueue([m = shared_from_this(), id]() {
        Manager::Impl::work(m, id);
    });

  } else if (id != connection::kInvalidId) {
    std::unique_lock<std::mutex> lock{impl_->mutex};
    auto it = impl_->wait_work_conn_ids.find(id);
    if (it != impl_->wait_work_conn_ids.end()) return;
    impl_->wait_work_conn_id_deque.push_back(id);
    impl_->wait_work_conn_ids.emplace(id);
  }
}
