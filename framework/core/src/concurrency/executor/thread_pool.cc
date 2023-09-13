#include "plain/concurrency/executor/thread_pool.h"
#include <semaphore>
#include <algorithm>
#include "plain/sys/thread.h"

using plain::concurrency::executor::detail::IdleWorkerSet;
using plain::concurrency::executor::detail::ThreadPoolWorker;
using plain::concurrency::executor::ThreadPool;

namespace plain::concurrency::executor::detail {

namespace {
  
struct thread_pool_per_thread_data {
  ThreadPoolWorker *this_worker;
  size_t this_thread_index;
  const size_t this_thread_hashed_id;

  static size_t calculate_hashed_id() noexcept {
    const auto this_thread_id = thread::get_current_virtual_id();
    const std::hash<size_t> hash;
    return hash(this_thread_id);
  }
  thread_pool_per_thread_data() noexcept :
    this_worker{nullptr}, this_thread_index{static_cast<size_t>(-1)},
    this_thread_hashed_id(calculate_hashed_id()) {
    // do nothing
  }
};

thread_local thread_pool_per_thread_data s_tl_thread_pool_data;

}

class alignas(kCacheInlineAlignment) ThreadPoolWorker {

 public:
  ThreadPoolWorker(
    ThreadPool &parent_pool, size_t index, size_t pool_size,
    std::chrono::milliseconds max_idle_time,
    const std::function<void(std::string_view)> &started_callback,
    const std::function<void(std::string_view)> &terminated_callback);
  ThreadPoolWorker(ThreadPoolWorker &&rhs) noexcept;
  ~ThreadPoolWorker() noexcept;

 public:
  void enqueue_foreign(Task &task);
  void enqueue_foreign(std::span<Task> tasks);
  void enqueue_foreign(
    std::deque<Task>::iterator begin, std::deque<Task>::iterator end);
  void enqueue_foreign(
    std::span<Task>::iterator begin, std::span<Task>::iterator end);

  void enqueue_local(Task &task);
  void enqueue_local(std::span<Task> tasks);

  void shutdown();

  std::chrono::milliseconds max_worker_idle_time() const noexcept;

  bool appears_empty() const noexcept;

 private:
  std::deque<Task> private_queue_;
  std::vector<size_t> idle_worker_list_;
  std::atomic_bool atomic_abort_;
  ThreadPool &parent_pool_;
  const size_t index_;
  const size_t pool_size_;
  const std::chrono::milliseconds max_idle_time_;
  const std::string worker_name_;
  alignas(kCacheInlineAlignment) std::mutex lock_;
  std::deque<Task> public_queue_;
  std::binary_semaphore semaphore_;
  bool idle_;
  bool abort_;
  std::atomic_bool task_found_or_abort_;
  thread_t thread_;
  const std::function<void(std::string_view thread_name)> started_callback_;
  const std::function<void(std::string_view thread_name)> terminated_callback_;

 private:
  void balance_work();

  bool wait_for_task(std::unique_lock<std::mutex> &lock);
  bool drain_queue_impl();
  bool drain_queue();

  void work_loop();

  void ensure_worker_active(
    bool first_enqueuer, std::unique_lock<std::mutex> &lock);

};

} // namespace plain::concurrency::executor::detail

IdleWorkerSet::IdleWorkerSet(size_t size) :
  approx_size_{0}, idle_flags_{std::make_unique<padded_flag[]>(size)},
  size_{size} {

}

void IdleWorkerSet::set_idle(size_t idle_thread) noexcept {
  const auto before = idle_flags_[idle_thread].flag.exchange(
    Status::Idle, std::memory_order_relaxed);
  if (before == Status::Idle) return;
  approx_size_.fetch_add(1, std::memory_order_relaxed);
}

void IdleWorkerSet::set_active(size_t idle_thread) noexcept {
  const auto before = idle_flags_[idle_thread].flag.exchange(
    Status::Active, std::memory_order_relaxed);
  if (before == Status::Active) return;
  approx_size_.fetch_sub(1, std::memory_order_relaxed);
}

bool IdleWorkerSet::try_acquire_flag(size_t index) noexcept {
  const auto worker_status =
    idle_flags_[index].flag.load(std::memory_order_relaxed);
  if (worker_status == Status::Active)
    return false;
  const auto before = idle_flags_[index].flag.exchange(
    Status::Active, std::memory_order_relaxed);
  const auto swapped = (before == Status::Idle);
  if (swapped)
    approx_size_.fetch_sub(1, std::memory_order_relaxed);
  return swapped;
}

size_t IdleWorkerSet::find_idle_worker(size_t caller_index) noexcept {
  if (approx_size_.load(std::memory_order_relaxed) <= 0)
    return static_cast<size_t>(-1);
  const auto starting_pos =
    (caller_index != static_cast<size_t>(-1)) ? caller_index : 
    (detail::s_tl_thread_pool_data.this_thread_hashed_id % size_);
  for (size_t i = 0; i < size_; ++i) {
    const auto index = (starting_pos + i) % size_;
    if (index == caller_index)
      continue;
    if (try_acquire_flag(index))
      return index;
  }
  return static_cast<size_t>(-1);
}

void IdleWorkerSet::find_idle_workers(
  size_t caller_index, std::vector<size_t> &result_buffer,
  size_t max_count) noexcept {
  assert(result_buffer.capacity() >= max_count);

  const auto approx_size = approx_size_.load(std::memory_order_relaxed);
  if (approx_size <= 0)
    return;

  assert(caller_index < size_);
  assert(caller_index == s_tl_thread_pool_data.this_thread_index);

  size_t count{0};
  const auto max_waiters = std::min(static_cast<size_t>(approx_size), max_count);
  for (size_t i = 0; (i < size_) && (count < max_waiters); ++i) {
    const auto index = (caller_index + i) % size_;
    if (index == caller_index)
      continue;
    if (try_acquire_flag(index)) {
      result_buffer.emplace_back(index);
      ++count;
    }
  }
}

ThreadPoolWorker::ThreadPoolWorker(
  ThreadPool &parent_pool, size_t index, size_t pool_size,
  std::chrono::milliseconds max_idle_time,
  const std::function<void(std::string_view)> &started_callback,
  const std::function<void(std::string_view)> &terminated_callback) :
  atomic_abort_{false}, parent_pool_{parent_pool}, index_{index},
  pool_size_{pool_size}, max_idle_time_{max_idle_time},
  worker_name_{detail::make_executor_worker_name(parent_pool.name_)},
  semaphore_{0}, idle_{false}, abort_{false}, task_found_or_abort_{false},
  started_callback_{started_callback},
  terminated_callback_{terminated_callback} {
  idle_worker_list_.reserve(pool_size);
}

ThreadPoolWorker::ThreadPoolWorker(ThreadPoolWorker &&rhs) noexcept :
  parent_pool_{rhs.parent_pool_}, index_{rhs.index_}, pool_size_{rhs.pool_size_},
  max_idle_time_{rhs.max_idle_time_}, semaphore_{0}, idle_{true}, abort_{true} {
  std::abort(); // What not use the delete the move copy construct?
}

ThreadPoolWorker::~ThreadPoolWorker() noexcept {
  assert(idle_);
  assert(!thread_.joinable());
}

void ThreadPoolWorker::balance_work() {
  const auto task_count = private_queue_.size();
  if (task_count < 2) return;
  const auto max_idle_worker_count = std::min(pool_size_ - 1, task_count - 1);
  if (max_idle_worker_count == 0) return;
  parent_pool_.find_idle_workers(
    index_, idle_worker_list_, max_idle_worker_count);
  const auto idle_count = idle_worker_list_.size();
  if (idle_count == 0) return;

  assert(idle_count <= task_count);
  const auto total_worker_count = (idle_count + 1);
  const auto donation_count = task_count / total_worker_count;
  auto extra = task_count - donation_count * total_worker_count;
  
  size_t begin{0};
  size_t end{donation_count};

  for (const auto idle_worker_index : idle_worker_list_) {
    assert(idle_worker_index != index_);
    assert(idle_worker_index < pool_size_);
    assert(begin < task_count);

    if (extra != 0) {
      ++end;
      --extra;
    }
    assert(end <= task_count);

    auto donation_begin_it = private_queue_.begin() + begin;
    auto donation_end_it = private_queue_.begin() + end;

    assert(donation_begin_it < private_queue_.end());
    assert(donation_end_it <= private_queue_.end());

    parent_pool_.worker_at(idle_worker_index).enqueue_foreign(
      donation_begin_it, donation_end_it);

    begin = end;
    end += donation_count;
  }

  assert(private_queue_.size() == task_count);

  assert(std::all_of(
    private_queue_.begin(), private_queue_.begin() + begin, [](auto &task) {
    return !static_cast<bool>(task);
  }));

  assert(std::all_of(
    private_queue_.begin() + begin, private_queue_.end(), [](auto &task) {
    return !static_cast<bool>(task);
  }));

  private_queue_.erase(private_queue_.begin(), private_queue_.begin() + begin);
  
  assert(!private_queue_.empty());

  idle_worker_list_.clear();
}

bool ThreadPoolWorker::wait_for_task(std::unique_lock<std::mutex> &lock) {
  assert(lock.owns_lock());
  if (!public_queue_.empty() || abort_) {
    return true;
  }

  lock.unlock();

  parent_pool_.mark_worker_idle(index_);

  auto event_found = false;
  const auto deadline = std::chrono::steady_clock::now() + max_idle_time_;

  while (true) {
    if (!semaphore_.try_acquire_until(deadline)) {
      if (std::chrono::steady_clock::now() <= deadline)
        continue;
      else
        break;
    }
    if (!task_found_or_abort_.load(std::memory_order_relaxed)) {
      continue;
    }
    lock.lock();
    if (public_queue_.empty() && !abort_) {
      lock.unlock();
      continue;
    }

    event_found = true;
    break;
  }

  if (!lock.owns_lock())
    lock.lock();
  if (!event_found || abort_) {
    idle_ = true;
    lock.unlock();
    return false;
  }
  assert(!public_queue_.empty());
  parent_pool_.mark_worker_active(index_);
  return true;
}

bool ThreadPoolWorker::drain_queue_impl() {
  auto aborted = false;

  while (!private_queue_.empty()) {
    balance_work();
    
    if (atomic_abort_.load(std::memory_order_relaxed)) {
      aborted = true;
      break;
    }

    assert(!private_queue_.empty());
    auto task = std::move(private_queue_.back());
    private_queue_.pop_back();
    task();
  }

  if (aborted) {
    std::unique_lock<decltype(lock_)> lock{lock_};
    idle_ = true;
    return false;
  }
  return true;
}

bool ThreadPoolWorker::drain_queue() {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (!wait_for_task(lock))
    return false;
  assert(lock.owns_lock());
  assert(!public_queue_.empty() || abort_);

  task_found_or_abort_.store(false, std::memory_order_relaxed);

  if (abort_) {
    idle_ = true;
    return false;
  }
  assert(private_queue_.empty());
  std::swap(private_queue_, public_queue_);
  lock.unlock();

  return drain_queue_impl();
}

void ThreadPoolWorker::work_loop() {
  s_tl_thread_pool_data.this_worker = this;
  s_tl_thread_pool_data.this_thread_index = index_;
  try {
    while (true) {
      if (!drain_queue())
        return;
    }
  } catch (const std::runtime_error &) {
    std::unique_lock<decltype(lock_)> lock{lock_};
    idle_ = true;
  }
}

void ThreadPoolWorker::ensure_worker_active(
  bool first_enqueuer, std::unique_lock<std::mutex> &lock) {
  assert(lock.owns_lock());

  if (!idle_) {
    lock.unlock();
    if (first_enqueuer)
      semaphore_.release();
    return;
  }
  auto stale_worker = std::move(thread_);
  thread_ = thread_t([this]{
    thread::set_name(worker_name_);
    started_callback_(worker_name_);
    work_loop();
    terminated_callback_(worker_name_);
  });
  idle_ = false;
  lock.unlock();
  if (stale_worker.joinable())
    stale_worker.join();
}

void ThreadPoolWorker::enqueue_foreign(Task &task) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (abort_)
    throw_runtime_shutdown_exception(parent_pool_.name_);
  task_found_or_abort_.store(true, std::memory_order_relaxed);
  
  const auto is_empty = public_queue_.empty();
  public_queue_.emplace_back(std::move(task));
  ensure_worker_active(is_empty, lock);
}

void ThreadPoolWorker::enqueue_foreign(std::span<Task> tasks) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (abort_)
    throw_runtime_shutdown_exception(parent_pool_.name_);

  task_found_or_abort_.store(true, std::memory_order_relaxed);

  const auto is_empty = public_queue_.empty();
  public_queue_.insert(
      public_queue_.end(), std::make_move_iterator(tasks.begin()),
      std::make_move_iterator(tasks.end()));
  ensure_worker_active(is_empty, lock);
}

void ThreadPoolWorker::enqueue_foreign(
  std::deque<Task>::iterator begin, std::deque<Task>::iterator end) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (abort_)
    throw_runtime_shutdown_exception(parent_pool_.name_);

  task_found_or_abort_.store(true, std::memory_order_relaxed);

  const auto is_empty = public_queue_.empty();
  public_queue_.insert(
    public_queue_.end(), std::make_move_iterator(begin),
    std::make_move_iterator(end));
  ensure_worker_active(is_empty, lock);
}

void ThreadPoolWorker::enqueue_foreign(
  std::span<Task>::iterator begin, std::span<Task>::iterator end) {
  std::unique_lock<decltype(lock_)> lock{lock_};
  if (abort_)
    throw_runtime_shutdown_exception(parent_pool_.name_);

  task_found_or_abort_.store(true, std::memory_order_relaxed);

  const auto is_empty = public_queue_.empty();
  public_queue_.insert(
    public_queue_.end(), std::make_move_iterator(begin),
    std::make_move_iterator(end));
  ensure_worker_active(is_empty, lock);
}

void ThreadPoolWorker::enqueue_local(Task &task) {
  if (atomic_abort_.load(std::memory_order_relaxed))
    throw_runtime_shutdown_exception(parent_pool_.name_);

  private_queue_.emplace_back(std::move(task));
}

void ThreadPoolWorker::enqueue_local(std::span<Task> tasks) {
   if (atomic_abort_.load(std::memory_order_relaxed))
    throw_runtime_shutdown_exception(parent_pool_.name_);
  private_queue_.insert(
    private_queue_.end(), std::make_move_iterator(tasks.begin()),
    std::make_move_iterator(tasks.end())); 
}

void ThreadPoolWorker::shutdown() {
  assert(!atomic_abort_.load(std::memory_order_relaxed));
  atomic_abort_.store(true, std::memory_order_relaxed);

  {
    std::unique_lock<decltype(lock_)> lock{lock_};
    abort_ = true;
  }

  task_found_or_abort_.store(true, std::memory_order_relaxed);
  
  semaphore_.release();

  if (thread_.joinable()) {
    thread_.join();
  }

  decltype(public_queue_) public_queue;
  decltype(private_queue_) private_queue;

  {
    std::unique_lock<decltype(lock_)> lock{lock_};
    public_queue = std::move(public_queue_);
    private_queue = std::move(private_queue_);
  }
  public_queue.clear();
  private_queue.clear();
}

std::chrono::milliseconds
ThreadPoolWorker::max_worker_idle_time() const noexcept {
  return max_idle_time_;
}

bool ThreadPoolWorker::appears_empty() const noexcept {
  return private_queue_.empty() &&
    !task_found_or_abort_.load(std::memory_order_relaxed);
}


ThreadPool::ThreadPool(
  std::string_view name, size_t size, std::chrono::milliseconds max_idle_time,
  const std::function<void(std::string_view)> &started_callback,
  const std::function<void(std::string_view)> &terminated_callback) :
  Derivable<ThreadPool>{name}, round_robin_cursor_{0}, idle_workers_{size},
  abort_{false} {

}

ThreadPool::~ThreadPool() = default;

void ThreadPool::find_idle_workers(
  size_t caller_index, std::vector<size_t> &buffer, size_t max_count) noexcept {
  idle_workers_.find_idle_workers(caller_index, buffer, max_count);
}

ThreadPoolWorker &ThreadPool::worker_at(size_t index) noexcept {
  assert(index <= workers_.size());
  return workers_[index];
}

void ThreadPool::mark_worker_idle(size_t index) noexcept {
  assert(index <= workers_.size());
  idle_workers_.set_idle(index);
}

void ThreadPool::mark_worker_active(size_t index) noexcept {
  assert(index <= workers_.size());
  idle_workers_.set_active(index);
}

void ThreadPool::enqueue(Task task) {
  const auto this_worker = detail::s_tl_thread_pool_data.this_worker;
  const auto this_worker_index = detail::s_tl_thread_pool_data.this_thread_index;
  if (this_worker != nullptr && this_worker->appears_empty()) {
    return this_worker->enqueue_local(task);
  }

  const auto idle_worker_pos = idle_workers_.find_idle_worker(this_worker_index);
  if (idle_worker_pos != static_cast<size_t>(-1))
    return workers_[idle_worker_pos].enqueue_foreign(task);
  if (this_worker != nullptr)
    return this_worker->enqueue_local(task);

  const auto next_worker = round_robin_cursor_.fetch_add(
    1, std::memory_order_relaxed) % workers_.size();
  workers_[next_worker].enqueue_foreign(task);
}

void ThreadPool::enqueue(std::span<Task> tasks) {
  if (detail::s_tl_thread_pool_data.this_worker != nullptr) {
    return detail::s_tl_thread_pool_data.this_worker->enqueue_local(tasks);
  }
  if (tasks.size() < workers_.size()) {
    for (auto &task : tasks) {
      enqueue(std::move(task));
    }
    return;
  }
  const auto task_count = tasks.size();
  const auto total_worker_count = workers_.size();
  const auto donation_count = task_count / total_worker_count;
  auto extra = task_count - donation_count * total_worker_count;

  size_t begin{0};
  size_t end{donation_count};
  for (size_t i = 0; i < total_worker_count; i++) {
    assert(begin < task_count);

    if (extra != 0) {
      ++end;
      --extra;
    }

    assert(end <= task_count);

    auto tasks_begin_it = tasks.begin() + begin;
    auto tasks_end_it = tasks.begin() + end;

    assert(tasks_begin_it < tasks.end());
    assert(tasks_end_it <= tasks.end());

    workers_[i].enqueue_foreign(tasks_begin_it, tasks_end_it);

    begin = end;
    end += donation_count;
  }
}

int32_t ThreadPool::max_concurrency_level() const noexcept {
  return static_cast<int32_t>(workers_.size());
}

bool ThreadPool::shutdown_requested() const {
  return abort_.load(std::memory_order_relaxed);
}

void ThreadPool::shutdown() {
  const auto abort = abort_.exchange(true, std::memory_order_relaxed);
  if (abort) return;
  for (auto &woker : workers_)
    woker.shutdown();
}

std::chrono::milliseconds ThreadPool::max_worker_idle_time() const noexcept {
  return workers_[0].max_worker_idle_time();
}
