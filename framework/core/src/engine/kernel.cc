#include "plain/engine/kernel.h"
#include "plain/basic/time.h"
#include "plain/basic/logger.h"
#include "plain/concurrency/executor/inline.h"
#include "plain/concurrency/executor/manual.h"
#include "plain/concurrency/executor/thread.h"
#include "plain/concurrency/executor/thread_pool.h"
#include "plain/concurrency/executor/worker_thread.h"
#include "plain/engine/timer_queue.h"

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



PLAIN_SINGLETON_DECL(plain::Kernel);
using plain::Kernel;

struct Kernel::Impl {
  std::shared_ptr<TimerQueue> timer_queue;
  std::shared_ptr<concurrency::executor::Inline> inline_executor;
  std::shared_ptr<concurrency::executor::ThreadPool> thread_pool_executor;
  std::shared_ptr<concurrency::executor::ThreadPool> background_executor;
  std::shared_ptr<concurrency::executor::Thread> thread_executor;
  detail::executor_collection registered_executors;
};

Kernel::Kernel() : Kernel(plain::engine_option{}) {

}

Kernel::Kernel(const engine_option &option) : impl_{std::make_unique<Impl>()} {
  using namespace plain::concurrency;
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

std::tuple<unsigned int, unsigned int, unsigned int>
Kernel::version() noexcept {
  return {2, 0, 0};
}
