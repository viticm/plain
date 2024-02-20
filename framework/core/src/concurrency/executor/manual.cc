#include "plain/concurrency/executor/manual.h"

using plain::concurrency::executor::Manual;

Manual::Manual() :
  Derivable<Manual>{"manual"}, abort_{false}, atomic_abort_{false} {

}

void Manual::enqueue(Task task) {
  std::unique_lock<decltype(lock_)> lock(lock_);
  if (abort_) {
    detail::throw_runtime_shutdown_exception(name_);
  }
  tasks_.emplace_back(std::move(task));
  lock.unlock();
  condition_.notify_all();
}

void Manual::enqueue(std::span<Task> tasks) {
   std::unique_lock<decltype(lock_)> lock(lock_);
  if (abort_) {
    detail::throw_runtime_shutdown_exception(name_);
  }
  tasks_.insert(
    tasks_.end(),
    std::make_move_iterator(tasks.begin()),
    std::make_move_iterator(tasks.end()));
  lock.unlock();
  condition_.notify_all(); 
}

int32_t Manual::max_concurrency_level() const noexcept {
  return std::numeric_limits<int>::max();
}

void Manual::shutdown() {
  const auto abort = atomic_abort_.exchange(true, std::memory_order_relaxed);  
  if (abort) return;
  decltype(tasks_) tasks;
  {
    std::unique_lock<decltype(lock_)> lock(lock_);
    abort_ = true;
    tasks = std::move(tasks_);
  }
  condition_.notify_all();
  tasks.clear();
}

bool Manual::shutdown_requested() const {
  return atomic_abort_.load(std::memory_order_relaxed);
}

size_t Manual::size() const {
  std::unique_lock<std::mutex> lock{lock_};
  return tasks_.size();
}

bool Manual::empty() const {
  return size() == 0;
}

size_t Manual::clear() {
  std::unique_lock<std::mutex> lock{lock_};
  if (abort_) {
    detail::throw_runtime_shutdown_exception(name_);
  }
  const auto tasks = std::move(tasks_);
  lock.unlock();
  return tasks.size();
}

bool Manual::loop_once() {
  return loop_impl(1) != 0;
}

bool Manual::loop_once_for(std::chrono::milliseconds max_waiting_time) {
  if (max_waiting_time == std::chrono::milliseconds(0))
    return loop_impl(1) != 0;
  return loop_until_impl(1, time_point_from_now(max_waiting_time));
}

size_t Manual::loop(size_t max_count) {
  return loop_impl(max_count);
}

size_t Manual::loop_for(
  size_t max_count, std::chrono::milliseconds max_waiting_time) {
  if (max_count == 0) return 0;
  if (max_waiting_time == std::chrono::milliseconds(0))
    return loop_impl(max_count);
  return loop_until_impl(max_count, time_point_from_now(max_waiting_time));
}

void Manual::wait_for_task() {
  wait_for_tasks_impl(1);
}

bool Manual::wait_for_task_for(std::chrono::milliseconds max_waiting_time) {
  return wait_for_tasks_impl(1, time_point_from_now(max_waiting_time)) == 1;
}

void Manual::wait_for_tasks(size_t count) {
  wait_for_tasks_impl(count);
}

size_t Manual::wait_for_tasks_for(
  size_t count, std::chrono::milliseconds max_waiting_time) {
  return wait_for_tasks_impl(count, time_point_from_now(max_waiting_time));
}

size_t Manual::loop_impl(size_t max_count) {
  if (max_count == 0) return 0;
  size_t executed{0};
  while (true) {
    if (executed == max_count)
      break;
    std::unique_lock<decltype(lock_)> lock{lock_};
    if (abort_)
      break;
    if (tasks_.empty())
      break;
    auto task = std::move(tasks_.front());
    tasks_.pop_front();
    lock.unlock();
    task();
    ++executed;
  }
  if (shutdown_requested())
    detail::throw_runtime_shutdown_exception(name_);

  return executed;
}

size_t Manual::loop_until_impl(
  size_t max_count,
  std::chrono::time_point<std::chrono::system_clock> deadline) {
  if (max_count == 0) return 0;
  size_t executed{0};
  deadline += std::chrono::milliseconds(1);
  while (true) {
    if (executed == max_count)
      break;
    const auto now = std::chrono::system_clock::now();
    if (now >= deadline)
      break;
    std::unique_lock<decltype(lock_)> lock{lock_};
    const auto found_task = condition_.wait_until(lock, deadline, [this] {
      return !tasks_.empty() || abort_;
    });
    if (abort_)
      break;
    if (!found_task)
      break;
    assert(!tasks_.empty());
    auto task = std::move(tasks_.front());
    tasks_.pop_front();
    lock.unlock();

    task();
    ++executed;
  }
  if (shutdown_requested())
    detail::throw_runtime_shutdown_exception(name_);
  return executed;
}

void Manual::wait_for_tasks_impl(size_t count) {
  if (count == 0) {
    if (shutdown_requested())
      detail::throw_runtime_shutdown_exception(name_);
    return;
  }
  std::unique_lock<decltype(lock_)> lock{lock_};
  condition_.wait(lock, [this, count] {
    return tasks_.size() >= count || abort_;
  });
  if (abort_)
    detail::throw_runtime_shutdown_exception(name_);
  assert(tasks_.size() >= count);
}

size_t Manual::wait_for_tasks_impl(
  size_t count, std::chrono::time_point<std::chrono::system_clock> deadline) {
  deadline += std::chrono::milliseconds(1);
  std::unique_lock<decltype(lock_)> lock{lock_};
  condition_.wait_until(lock, deadline, [this, count] {
    return tasks_.size() >= count || abort_;
  });
  if (abort_)
    detail::throw_runtime_shutdown_exception(name_);
  return tasks_.size();
}
