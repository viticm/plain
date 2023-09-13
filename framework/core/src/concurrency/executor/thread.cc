#include "plain/concurrency/executor/thread.h"

using plain::concurrency::executor::Thread;

Thread::Thread(
  const std::function<void(std::string_view)> &started_callback,
  const std::function<void(std::string_view)> &terminated_callback) :
  Derivable<Thread>{"thread"}, abort_{false}, atomic_abort_{false},
  started_callback_{started_callback},
  terminated_callback_{terminated_callback} {

}

Thread::~Thread() noexcept {
  assert(workers_.empty());
  assert(last_retired_.empty());
}

void Thread::enqueue_impl(std::unique_lock<std::mutex> &lock, Task &task) {
  assert(lock.owns_lock());
  auto &new_thread = workers_.emplace_front();
  new_thread = thread_t(
    [this, self_it = workers_.begin(), task = std::move(task)]() mutable {
    std::string name = detail::make_executor_worker_name(name_);
    if (static_cast<bool>(started_callback_)) {
      started_callback_(name);
    }
    task();
    retire_worker(self_it);
    if (static_cast<bool>(terminated_callback_)) {
      terminated_callback_(name);
    }
  });
}

void Thread::enqueue(Task task) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (abort_)
    detail::make_executor_worker_name(name_);
  enqueue_impl(lock, task);
}

void Thread::enqueue(std::span<Task> tasks) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (abort_)
    detail::make_executor_worker_name(name_);
  for (auto &task : tasks)
    enqueue_impl(lock, task);
}

int32_t Thread::max_concurrency_level() const noexcept {
  return std::numeric_limits<int>::max();
}

bool Thread::shutdown_requested() const {
  return atomic_abort_.load(std::memory_order_relaxed);
}

void Thread::shutdown() {
  const auto abort = atomic_abort_.exchange(true, std::memory_order_relaxed);
  if (abort) return; // Shutdown started.
  std::unique_lock<decltype(lock_)> lock{lock_};
  abort_ = true;
  condition_.wait(lock, [this] {
    return workers_.empty();
  });
  if (last_retired_.empty())
    return;
  assert(last_retired_.size() == 1);
  last_retired_.front().join();
  last_retired_.clear();
}

void Thread::retire_worker(std::list<thread_t>::iterator it) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  auto last_retired = std::move(last_retired_);
  last_retired_.splice(last_retired_.begin(), workers_, it);
  lock.unlock();

  condition_.notify_one();
  if (last_retired.empty()) return;
  assert(last_retired.size() == 1);
  last_retired.front().join();
}
