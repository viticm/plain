#include "plain/concurrency/executor/worker_thread.h"

namespace plain::concurrency::executor{

namespace detail {

static thread_local WorkerThread *s_tl_this_worker{nullptr};

} // namespace detail

} // namespace plain::concurrency::executor

using plain::concurrency::executor::WorkerThread;

WorkerThread::WorkerThread(
  const std::function<void(std::string_view)> &started_callback,
  const std::function<void(std::string_view)> &terminated_callback) :
  Derivable<WorkerThread>{"worker thread"}, private_atomic_abort_{false},
  semaphore_{0}, abort_{false},
  started_callback_{started_callback},
  terminated_callback_{terminated_callback} {

}

void WorkerThread::make_os_worker_thread() {
  thread_ = thread_t([this]{
    auto name = detail::make_executor_worker_name(name_);
    thread::set_name(name);
    started_callback_(name);
    work_loop();
    terminated_callback_(name);
  });
}

bool WorkerThread::drain_queue_impl() {
  while (!private_queue_.empty()) {
    auto task = std::move(private_queue_.front());
    private_queue_.pop_front();
    if (private_atomic_abort_.load(std::memory_order_relaxed)) {
      return false;
    }
    task();
  }
  return true;
}

void WorkerThread::wait_for_task(std::unique_lock<std::mutex> &lock) {
  assert(lock.owns_lock());
  if (!public_queue_.empty() || abort_)
    return;

  while (true) {
    lock.unlock();

    semaphore_.acquire();

    if (!public_queue_.empty() || abort_)
      break;
  }
}

bool WorkerThread::drain_queue() {
  std::unique_lock<decltype(lock_)> lock(lock_);
  
  wait_for_task(lock);

  assert(lock.owns_lock());
  assert(!public_queue_.empty() || abort_);

  if (abort_) return false;

  assert(private_queue_.empty());
  std::swap(private_queue_, public_queue_);
  lock.unlock();

  return drain_queue_impl();
}

void WorkerThread::work_loop() {
  detail::s_tl_this_worker = this;
  while (true) {
    if (!drain_queue()) break;
  }
}

void WorkerThread::enqueue_local(Task &task) {
  if (private_atomic_abort_.load(std::memory_order_relaxed))
    detail::throw_runtime_shutdown_exception(name_);
  private_queue_.emplace_back(std::move(task));
}

void WorkerThread::enqueue_local(std::span<Task> tasks) {
  if (private_atomic_abort_.load(std::memory_order_relaxed))
    detail::throw_runtime_shutdown_exception(name_);
  private_queue_.insert(
    private_queue_.end(),
    std::make_move_iterator(tasks.begin()),
    std::make_move_iterator(tasks.end()));
}

void WorkerThread::enqueue_foreign(Task &task) {
  std::unique_lock<decltype(lock_)> lock(lock_);
  if (abort_)
    detail::throw_runtime_shutdown_exception(name_);

  const auto is_empty = public_queue_.empty();
  public_queue_.emplace_back(std::move(task));

  if (!thread_.joinable())
    return make_os_worker_thread();

  lock.unlock();
  if (is_empty)
    semaphore_.release();
}

void WorkerThread::enqueue_foreign(std::span<Task> tasks) {
  std::unique_lock<decltype(lock_)> lock(lock_);
  if (abort_)
    detail::throw_runtime_shutdown_exception(name_);

  const auto is_empty = public_queue_.empty();
  public_queue_.insert(
    public_queue_.end(),
    std::make_move_iterator(tasks.begin()),
    std::make_move_iterator(tasks.end()));
  if (!thread_.joinable())
    return make_os_worker_thread();

  lock.unlock();
  if (is_empty)
    semaphore_.release();
}

void WorkerThread::enqueue(Task task) {
  if (detail::s_tl_this_worker == this)
    enqueue_local(task);

  enqueue_foreign(task);
}

void WorkerThread::enqueue(std::span<Task> tasks) {
  if (detail::s_tl_this_worker == this)
    enqueue_local(tasks);

  enqueue_foreign(tasks);
}

int32_t WorkerThread::max_concurrency_level() const noexcept {
  return 1;
}

bool WorkerThread::shutdown_requested() const {
  return atomic_abort_.load(std::memory_order_relaxed);
}

void WorkerThread::shutdown() {
  const auto abort = atomic_abort_.exchange(true, std::memory_order_relaxed);
  if (abort) return;

  {
    std::unique_lock<decltype(lock_)> lock(lock_);
    abort_ = true;
  }
  private_atomic_abort_.store(true, std::memory_order_relaxed);
  semaphore_.release();

  if (thread_.joinable())
    thread_.join();

  decltype(private_queue_) private_queue;
  decltype(public_queue_) public_queue;
  {
    std::unique_lock<decltype(lock_)> lock(lock_);
    private_queue = std::move(private_queue_);
    public_queue = std::move(public_queue_);
  }

  private_queue.clear();
  public_queue.clear();
}
