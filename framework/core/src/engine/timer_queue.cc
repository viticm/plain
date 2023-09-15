#include "plain/engine/timer_queue.h"
#include <set>
#include "plain/concurrency/executor/basic.h"
#include "plain/concurrency/result/lazy.h"
#include "plain/sys/thread.h"

using plain::TimerQueue;
using clock_type = std::chrono::high_resolution_clock;
using time_point_t = std::chrono::time_point<clock_type>;
using timer_ptr_t = std::shared_ptr<plain::detail::TimerStateBasic>;

namespace plain::detail {
enum class TimerRequest { Add, Remove };
} // namespace plain::detail
using _timer_t = std::shared_ptr<plain::detail::TimerStateBasic>;
using request_queue_t = 
  std::vector<std::pair<_timer_t, plain::detail::TimerRequest>>;

namespace plain::detail {

namespace {
struct deadline_comparator {
  bool operator()(const timer_ptr_t &a, const timer_ptr_t &b) const noexcept {
    return a->get_deadline() < b->get_deadline();
  }
};

class TimerQueueInternal {

 public:
  using timer_set_t = std::multiset<timer_ptr_t, deadline_comparator>;
  using timer_set_iterator_t = typename timer_set_t::iterator;
  using map_iterator_t = std::unordered_map<timer_ptr_t, timer_set_iterator_t>;

 public:
  bool empty() const noexcept {
    assert(iterator_mapper_.size() == timers_.size());
    return timers_.empty();
  }

  ::time_point_t process_timers(request_queue_t &queue) {
    process_request_queue(queue);

    const auto now = std::chrono::high_resolution_clock::now();
    while (true) {
      if (timers_.empty()) break;
      timer_set_t temp_set;

      auto first_timer_it = timers_.begin();
      auto timer_ptr = *first_timer_it;
      const auto is_oneshot = timer_ptr->is_oneshot();

      if (!timer_ptr->expired(now)) break;

      auto timer_node = timers_.extract(first_timer_it);

      auto temp_it = temp_set.insert(std::move(timer_node));

      const auto cancelled = timer_ptr->cancelled();
      if (!cancelled)
        (*temp_it)->fire();

      if (is_oneshot || cancelled) {
        iterator_mapper_.erase(timer_ptr);
        continue;
      }
      timer_node = temp_set.extract(temp_it);
      auto new_it = timers_.insert(std::move(timer_node));
      assert(iterator_mapper_.find(timer_ptr) != iterator_mapper_.end());
      iterator_mapper_[timer_ptr] = new_it;
    }
    if (timers_.empty()) {
      reset_containers_memory();
      return now + std::chrono::hours(24);
    }
    return (**timers_.begin()).get_deadline();
  }

 private:
  timer_set_t timers_;
  map_iterator_t iterator_mapper_;

 private:
  void add_timer_internal(timer_ptr_t new_timer) {
    assert(iterator_mapper_.find(new_timer) == iterator_mapper_.end());
    auto timer_it = timers_.emplace(new_timer);
    iterator_mapper_.emplace(std::move(new_timer), timer_it);
  }

  void remove_timer_internal(timer_ptr_t existing_timer) {
    auto timer_it = iterator_mapper_.find(existing_timer);
    if (timer_it == iterator_mapper_.end()) {
      assert(existing_timer->is_oneshot() || existing_timer->cancelled());
      return;
    }
    auto set_iterator = timer_it->second;
    timers_.erase(set_iterator);
    iterator_mapper_.erase(timer_it);
  }

  void process_request_queue(request_queue_t &queue) {
    for (auto &request : queue) {
      auto &timer_ptr = request.first;
      const auto opt = request.second;
      if (opt == TimerRequest::Add) {
        add_timer_internal(std::move(timer_ptr));
      } else {
        remove_timer_internal(std::move(timer_ptr));
      }
    }
  }

  void reset_containers_memory() noexcept {
    assert(empty());
    timer_set_t timers;
    std::swap(timers_, timers);
    map_iterator_t iterator_mapper;
    std::swap(iterator_mapper_, iterator_mapper);
  }

};

}

} // namespace plain::detail

struct TimerQueue::Impl {
  std::atomic_bool atomic_abort;
  request_queue_t request_queue;
  thread_t worker;
  std::condition_variable condition;
  bool abort;
  bool idle;
  std::mutex lock;
  const std::chrono::milliseconds max_waiting_time;
  const std::function<void(std::string_view)> started_callback;
  const std::function<void(std::string_view)> terminated_callback;
  
  Impl(
    std::chrono::milliseconds max_waiting_time,
    const std::function<void(std::string_view name)> &started_callback,
    const std::function<void(std::string_view name)> &terminated_callback);

  thread_t ensure_worker_thread(std::unique_lock<std::mutex> &lock);
  
  void add_internal_timer(
    std::unique_lock<std::mutex> &lock, _timer_t new_timer);
  
  concurrency::LazyResult<void> make_delay_object_impl(
    std::chrono::milliseconds due_time,
    std::shared_ptr<TimerQueue> self,
    std::shared_ptr<concurrency::executor::Basic> executor);

  void work_loop();
};

TimerQueue::Impl::Impl(
  std::chrono::milliseconds max_waiting_time,
  const std::function<void(std::string_view name)> &started_callback,
  const std::function<void(std::string_view name)> &terminated_callback) :
  atomic_abort{false}, abort{false}, idle{true},
  max_waiting_time{max_waiting_time},
  started_callback{started_callback},
  terminated_callback{terminated_callback} {

}

plain::thread_t TimerQueue::Impl::ensure_worker_thread(
  std::unique_lock<std::mutex> &lock) {
  assert(lock.owns_lock());
  if (!idle) return {};
  auto old_worker = std::move(worker);
  worker = thread_t([this] {
    auto name = 
      concurrency::executor::detail::make_executor_worker_name("timer queue");
    thread::set_name(name);
    started_callback(name);
    work_loop();
    terminated_callback(name);
  });
  idle = false;
  return old_worker;
}

void TimerQueue::Impl::add_internal_timer(
    std::unique_lock<std::mutex> &_lock, _timer_t new_timer) {
  assert(_lock.owns_lock());
  request_queue.emplace_back(std::move(new_timer), detail::TimerRequest::Add);
  _lock.unlock();
  condition.notify_one();
}

plain::concurrency::LazyResult<void> TimerQueue::Impl::make_delay_object_impl(
  std::chrono::milliseconds due_time,
  std::shared_ptr<TimerQueue> self,
  std::shared_ptr<concurrency::executor::Basic> executor) {
  class delay_object_awaitable : public concurrency::suspend_always {
   public:
    delay_object_awaitable(
      size_t due_time_ms,
      TimerQueue &parent_queue,
      std::shared_ptr<concurrency::executor::Basic> executor) noexcept :
      due_time_ms_{due_time_ms},
      parent_queue_{parent_queue},
      executor_{std::move(executor)} {

    }

    void await_suspend(concurrency::coroutine_handle<void> handle) noexcept {
      try {
        parent_queue_.make_timer_impl(
          due_time_ms_, 0, std::move(executor_), true,
          concurrency::result::detail::AwaitViaFunctor{handle, &interrupted_});
      } catch (...) {
        // do nothing.
      }
    }

    void await_resume() const {
      if (interrupted_)
        throw std::runtime_error("associated task was interrupted abnormally");
    }

   private:
    const size_t due_time_ms_;
    TimerQueue &parent_queue_;
    std::shared_ptr<concurrency::executor::Basic> executor_;
    bool interrupted_{false};
  };

  // FIXME: co_await with use return type from promise(see result/promise.h)
  co_await delay_object_awaitable{
    static_cast<size_t>(due_time.count()), *self, std::move(executor)};
}

void TimerQueue::Impl::work_loop() {
  time_point_t next_deadline;
  detail::TimerQueueInternal internal_state;

  while (true) {
    std::unique_lock<decltype(lock)> _lock(lock);
    if (internal_state.empty()) {
      const auto res = condition.wait_for(_lock, max_waiting_time, [this] {
        return !request_queue.empty() || abort;
      });
      if (!res) {
        idle = true;
        _lock.unlock();
        return;
      }
    } else {
      condition.wait_until(_lock, next_deadline, [this] {
        return !request_queue.empty() || abort;
      });
    }

    if (abort) return;

    auto _request_queue = std::move(request_queue);
    _lock.unlock();

    next_deadline = internal_state.process_timers(request_queue);
    const auto now = clock_type::now();
    if (next_deadline <= now)
       continue;
  }
}

TimerQueue::TimerQueue(
  std::chrono::milliseconds max_waiting_time,
  const std::function<void(std::string_view name)> &started_callback,
  const std::function<void(std::string_view name)> &terminated_callback) :
  impl_{std::make_unique<Impl>(
    max_waiting_time, started_callback, terminated_callback)} {

}

TimerQueue::~TimerQueue() noexcept {

}

void TimerQueue::remove_internal_timer(
    std::shared_ptr<detail::TimerStateBasic> existing_timer) {
  {
    std::unique_lock<decltype(impl_->lock)> lock(impl_->lock);
    impl_->request_queue.emplace_back(
      std::move(existing_timer), detail::TimerRequest::Remove);
  }
  impl_->condition.notify_one();
}

std::mutex &TimerQueue::get_lock() {
  return impl_->lock;
}

void TimerQueue::shutdown() {
  const auto state_before =
    impl_->atomic_abort.exchange(true, std::memory_order_relaxed);
  if (state_before) return;
  std::unique_lock<std::mutex> lock(impl_->lock);
  impl_->abort = true;
  if (!impl_->worker.joinable())
    return;
  impl_->request_queue.clear();
  lock.unlock();

  impl_->condition.notify_all();
  impl_->worker.join();
}
  
bool TimerQueue::shutdown_requested() const noexcept {
  return impl_->atomic_abort.load(std::memory_order_relaxed);
}
  
std::chrono::milliseconds TimerQueue::max_worker_idle_time() const noexcept {
  return impl_->max_waiting_time;
}

plain::concurrency::LazyResult<void> TimerQueue::make_delay_object(
  std::chrono::milliseconds due_time,
  std::shared_ptr<concurrency::executor::Basic> executor) {
  if (!static_cast<bool>(executor))
    throw std::runtime_error("executor is null");
  return impl_->make_delay_object_impl(
    due_time, shared_from_this(), std::move(executor));
}

void TimerQueue::add_timer(
  std::unique_lock<std::mutex> &lock,
  std::shared_ptr<detail::TimerStateBasic> new_timer) {
  assert(lock.owns_lock());

  if (impl_->abort)
    throw std::runtime_error("timer queue has been shut down");

  auto old_thread = impl_->ensure_worker_thread(lock);
  impl_->add_internal_timer(lock, new_timer);
  if (old_thread.joinable()) old_thread.join();
}
