#include "plain/engine/kernel.h"
#include <map>
#include "plain/basic/type/byte.h"
#include "plain/basic/time.h"
#include "plain/basic/logger.h"
#include "plain/basic/utility.h"
#include "plain/concurrency/executor/inline.h"
#include "plain/concurrency/executor/manual.h"
#include "plain/concurrency/executor/thread.h"
#include "plain/concurrency/executor/thread_pool.h"
#include "plain/concurrency/executor/worker_thread.h"
#include "plain/engine/timer_queue.h"
#include "plain/net/connection/basic.h"
#include "plain/net/packet/basic.h"
#include "plain/net/connection/manager.h"
#include "plain/net/address.h"
#include "plain/net/listener.h"
#include "plain/net/utility.h"

namespace plain::detail {
namespace {

size_t default_max_cpu_workers() noexcept {
  return static_cast<size_t>(thread::hardware_concurrency() * 1);
}

size_t default_max_background_workers() noexcept {
  return static_cast<size_t>(thread::hardware_concurrency() * 4);
}

constexpr auto kDefaultMaxWorkerWaitTime = std::chrono::seconds(60 * 2);

struct executor_collection {
  std::mutex lock;
  std::vector<std::shared_ptr<concurrency::executor::Basic>> executors;
  void register_executor(
    std::shared_ptr<concurrency::executor::Basic> executor) {
    assert(static_cast<bool>(executor));
    std::unique_lock<decltype(lock)> _lock{lock};
    assert(
      std::find(executors.begin(), executors.end(), executor) == executors.end());
    executors.emplace_back(std::move(executor));
  }
  
  void shutdown_all() {
    std::unique_lock<decltype(lock)> _lock{lock};
    for (auto &executor : executors) {
      assert(static_cast<bool>(executor));
      executor->shutdown();
    }
    executors = {};
  }
};

}
}

using plain::engine_option;
using plain::TimerQueue;

engine_option::engine_option() noexcept :
  max_cpu_threads{detail::default_max_cpu_workers()},
  max_thread_pool_executor_waiting_time{detail::kDefaultMaxWorkerWaitTime},
  max_background_threads{detail::default_max_background_workers()},
  max_background_executor_waiting_time{detail::kDefaultMaxWorkerWaitTime},
  max_timer_queue_waiting_time{std::chrono::seconds(60 * 2)} {

}



using plain::Kernel;

struct Kernel::Impl {
  std::shared_ptr<TimerQueue> timer_queue;
  std::shared_ptr<concurrency::executor::Inline> inline_executor;
  std::shared_ptr<concurrency::executor::ThreadPool> thread_pool_executor;
  std::shared_ptr<concurrency::executor::ThreadPool> background_executor;
  std::shared_ptr<concurrency::executor::Thread> thread_executor;
  detail::executor_collection registered_executors;
  std::unique_ptr<net::Listener> console_listener;
  std::map<std::string, std::weak_ptr<net::connection::Manager>> nets;
  std::map<std::string, console_func> console_handlers;
  std::map<std::string, std::string> console_descs;
  uint32_t unnamed_net_count{0};
  std::mutex mutex;

  static std::string console_cmd_list(const std::vector<std::string> &args);
  static std::string console_cmd_kill(const std::vector<std::string> &args);
  static std::string console_cmd_help(const std::vector<std::string> &args);
};

std::string Kernel::Impl::console_cmd_list(
  const std::vector<std::string> &args) {
  UNUSED(args);
  std::unique_lock<decltype(ENGINE->impl_->mutex)>
    auto_lock{ENGINE->impl_->mutex};
  std::string r;
  r += "Total: " + std::to_string(ENGINE->impl_->nets.size());
  if (!ENGINE->impl_->nets.empty()) {
    r += LF;
    for (auto it : ENGINE->impl_->nets) {
      auto net = it.second.lock();
      if (net) {
        r += "\t";
        r += net->setting_.name;
        r += " count: " + std::to_string(net->size());
        r += " maxcount: " + std::to_string(net->setting_.max_count);
        r += " send: " + format_size(net->send_size());
        r += " recv: " + format_size(net->recv_size());
        r += " address: " + net->setting_.address;
        r += " mode: " + net::get_mode_name(net->setting_.mode);
        r += LF;
      }
    }
  }
  // Remove the last '\n'
  if (r.back() == '\n') {
    r.pop_back();
  }
  if (r.back() == '\r') {
    r.pop_back();
  }
  return r;
}

std::string Kernel::Impl::console_cmd_kill(
  const std::vector<std::string> &args) {
  std::string r{"ok"};
  decltype(ENGINE->impl_->nets) nets;
  {
    std::unique_lock<decltype(ENGINE->impl_->mutex)>
      auto_lock{ENGINE->impl_->mutex};
    nets = ENGINE->impl_->nets;
  }
  if (nets.empty()) return "failed";
  if (args.empty()) {
    // killall
    for (auto it : nets) {
      auto net = it.second.lock();
      if (net) net->stop();
    }
  } else {
    std::string fails;
    for (const auto &name : args) {
      auto it = nets.find(name);
      if (it == nets.end() || name == "console") {
        fails += name + " ";
      } else {
        auto net = it->second.lock();
        if (net) {
          net->stop();
        } else { // Clear from list.
          std::unique_lock<decltype(ENGINE->impl_->mutex)>
            auto_lock{ENGINE->impl_->mutex};
          ENGINE->impl_->nets.erase(name);
        }
      }
    }
    if (!fails.empty())
      r = fails + "fails";
  }
  return r;
}

std::string Kernel::Impl::console_cmd_help(
  const std::vector<std::string> &args) {
  std::string r;
  if (args.empty()) {
    for (const auto &it : ENGINE->impl_->console_descs) {
      r += it.first;
      r += "\t";
      r += it.second;
      r += LF;
    }
  } else {
    for (const auto &name : args) {
      auto it = ENGINE->impl_->console_descs.find(name);
      if (it != ENGINE->impl_->console_descs.end()) {
        r += it->first;
        r += "\t";
        r += it->second;
        r += LF;
      }
    }
  }
  if (!r.empty() && r.back() == '\n')
    r.pop_back();
  if (!r.empty() && r.back() == '\r')
    r.pop_back();
  return r;
}

Kernel::Kernel() : Kernel(plain::engine_option{}) {

}

Kernel::Kernel(const engine_option &option) : impl_{std::make_unique<Impl>()} {
  using namespace plain::concurrency;

  impl_->timer_queue = std::make_shared<TimerQueue>(
    option.max_timer_queue_waiting_time,
    option.thread_started_callback, option.thread_terminated_callback);

  impl_->inline_executor = std::make_shared<executor::Inline>();
  impl_->registered_executors.register_executor(impl_->inline_executor);

  impl_->thread_pool_executor = std::make_shared<executor::ThreadPool>(
    "thread pool executor", option.max_cpu_threads, 
    option.max_thread_pool_executor_waiting_time,
    option.thread_started_callback,
    option.thread_terminated_callback
  );
  impl_->registered_executors.register_executor(impl_->thread_pool_executor);

  impl_->background_executor = std::make_shared<executor::ThreadPool>(
    "thread background executor", option.max_cpu_threads, 
    option.max_thread_pool_executor_waiting_time,
    option.thread_started_callback,
    option.thread_terminated_callback
  );
  impl_->registered_executors.register_executor(impl_->background_executor);

  impl_->thread_executor = std::make_shared<executor::Thread>(
    option.thread_started_callback, option.thread_terminated_callback);
  impl_->registered_executors.register_executor(impl_->thread_executor);

  register_console_handler("list", Impl::console_cmd_list, "Show net list");
  register_console_handler(
    "kill", Impl::console_cmd_kill, "Kill net from list(kill name1 name2 ...)");
  register_console_handler("killall", Impl::console_cmd_kill, "Kill all net");
  register_console_handler(
    "help", Impl::console_cmd_help,
    "Show help to use commands, help command1 command2 (empty will show all)");
}
  
Kernel::~Kernel() noexcept {
  try {
    impl_->timer_queue->shutdown();
    impl_->registered_executors.shutdown_all();
  } catch (...) {
    std::abort();
  }
}

std::shared_ptr<TimerQueue> Kernel::timer_queue() const noexcept {
  return impl_->timer_queue;
}
  
std::shared_ptr<plain::concurrency::executor::Inline>
Kernel::inline_executor() const noexcept {
  return impl_->inline_executor;
}
  
std::shared_ptr<plain::concurrency::executor::ThreadPool>
Kernel::thread_pool_executor() const noexcept {
  return impl_->thread_pool_executor;
}
  
std::shared_ptr<plain::concurrency::executor::ThreadPool>
Kernel::background_executor() const noexcept {
  return impl_->background_executor;
}
  
std::shared_ptr<plain::concurrency::executor::Thread>
Kernel::thread_executor() const noexcept {
  return impl_->thread_executor;
}

std::shared_ptr<plain::concurrency::executor::WorkerThread>
Kernel::make_worker_thread_executor() {
  using plain::concurrency::executor::WorkerThread;
  auto executor = std::make_shared<WorkerThread>();
  impl_->registered_executors.register_executor(executor);
  return executor;
}

std::shared_ptr<plain::concurrency::executor::Manual>
Kernel::make_manual_executor() {
  using plain::concurrency::executor::Manual;
  auto executor = std::make_shared<Manual>();
  impl_->registered_executors.register_executor(executor);
  return executor;
}

std::tuple<uint32_t, uint32_t, uint32_t>
Kernel::version() noexcept {
  return {2, 0, 0};
}

void Kernel::add(std::shared_ptr<net::connection::Manager> net) {
  std::unique_lock<decltype(impl_->mutex)> auto_lock{impl_->mutex};
  auto name = net->setting_.name;
  if (name.empty()) { // Set unnamed
    name = "unknown" + std::to_string(++impl_->unnamed_net_count);
    net->setting_.name = name;
  }
  auto it = impl_->nets.find(name);
  if (it != impl_->nets.end())
    throw std::runtime_error("repeated net: " + name);
  impl_->nets.emplace(name, net);
}
   
void Kernel::remove(std::shared_ptr<net::connection::Manager> net) noexcept {
  std::unique_lock<decltype(impl_->mutex)> auto_lock{impl_->mutex};
  auto name = net->setting_.name;
  impl_->nets.erase(name);
}
   
void Kernel::remove_net(std::string_view name) noexcept {
  std::unique_lock<decltype(impl_->mutex)> auto_lock{impl_->mutex};
  impl_->nets.erase(name.data());
}

bool Kernel::enable_console(std::string_view addr) noexcept {
  if (static_cast<bool>(impl_->console_listener))
    return false;
  net::setting_t setting;
  setting.address = addr;
  setting.name = "console";
  impl_->console_listener = std::make_unique<net::Listener>(setting);
  if (!impl_->console_listener->start())
    return false;
  LOG_INFO << "console listen on " <<
    impl_->console_listener->address().text();
  
  impl_->console_listener->set_dispatcher([](
    net::connection::Basic *conn, std::shared_ptr<net::packet::Basic> packet){
    auto d = reinterpret_cast<const char *>(packet->data().data());
    std::vector<std::string> params;
    explode(d, params, " ", true, true);
    if (params.empty()) return true;
    auto cmd = params[0];
    auto it = ENGINE->impl_->console_handlers.find(cmd);
    if (it == ENGINE->impl_->console_handlers.end()) return false;
    params.erase(params.begin());
    auto r = it->second(params);
    if (!r.empty()) {
      auto p = std::make_shared<net::packet::Basic>();
      p->set_writeable(true);
      r += LF;
      p->write(as_const_bytes(r));
      p->set_writeable(false);
      conn->send(p);
    }
    return true;
  });
  
  impl_->console_listener->set_codec(
    {.encode = net::stream::line_encode, .decode = net::stream::line_decode});

  return true;
}

void Kernel::register_console_handler(
  std::string_view cmd, console_func func, std::string_view desc) noexcept {
  std::unique_lock<decltype(impl_->mutex)> auto_lock{impl_->mutex};
  auto it = impl_->console_handlers.find(cmd.data());
  if (it != impl_->console_handlers.end())
    LOG_WARN << "cmd: " << cmd.data() << " handler will replace";
  impl_->console_handlers.emplace(cmd, func);
  if (!desc.empty())
    impl_->console_descs.emplace(cmd, desc);
}
