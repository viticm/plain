#include "plain/sys/thread.h"
#include "plain/basic/logger.h"

namespace plain {

namespace thread {

namespace detail {

namespace {

std::uintptr_t generate_thread_id() noexcept {
  static std::atomic_uintptr_t s_id_seed = 1;
  return s_id_seed.fetch_add(1, std::memory_order_relaxed);
}

struct thread_per_thread_data {
  const std::uintptr_t id = generate_thread_id();
};

thread_local thread_per_thread_data s_tl_thread_per_data;

}

} // namespace detail

void set_name(const std::string_view &name) noexcept {
#if OS_WIN
  const std::wstring utf16_name(name.begin(), name.end());
  ::SetThreadDescription(::GetCurrentThread(), utf16_name.data());
#elif OS_UNIX
  ::pthread_setname_np(::pthread_self(), name.data());
#elif OS_MAC
  ::pthread_setname_np(name.data());
#endif
}

std::uintptr_t get_current_virtual_id() noexcept {
  return detail::s_tl_thread_per_data.id;
}

} // namespace thread

std::atomic<int32_t> ThreadCollect::count_{0};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads)
  : stop_(false) {
  for(size_t i = 0; i < threads; ++i)
    workers_.emplace_back(
      [this] {
        for(;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            this->condition_.wait(lock,
              [this]{ return this->stop_ || !this->tasks_.empty(); });
            if(this->stop_ && this->tasks_.empty())
              return;
            task = std::move(this->tasks_.front());
            this->tasks_.pop();
          }
          task();
        }
      }
    );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
  -> std::future<typename std::invoke_result_t<F, Args...>> {
  using return_type = typename std::invoke_result_t<F, Args...>;
  auto task = std::make_shared< std::packaged_task<return_type()> >(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // don't allow enqueueing after stopping the pool
    if (stop_)
      throw std::runtime_error("enqueue on stopped ThreadPool");

    tasks_.emplace([task](){ (*task)(); });
  }
  condition_.notify_one();
  return res;
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }
  condition_.notify_all();
  for (auto& worker : workers_)
    worker.join();
}

} // namespace plain
