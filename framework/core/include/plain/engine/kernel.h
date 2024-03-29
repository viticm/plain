/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id kernel.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/01 19:49
 * @uses The plain engine kernel class.
 */

#ifndef PLAIN_ENGINE_KERNEL_H_
#define PLAIN_ENGINE_KERNEL_H_

#include "plain/engine/config.h"
#include "plain/basic/singleton.h"
#include "plain/basic/logger.h"
#include "plain/concurrency/config.h"
#include "plain/sys/thread.h"
#include "plain/net/config.h"

namespace plain {

class PLAIN_API Kernel {

 public:
  void start();

 public:
  Kernel();
  Kernel(const engine_option &option);
  ~Kernel() noexcept;

 public:
  std::shared_ptr<TimerQueue> timer_queue() const noexcept;
  
  std::shared_ptr<concurrency::executor::Inline>
  inline_executor() const noexcept;
  
  std::shared_ptr<concurrency::executor::ThreadPool>
  thread_pool_executor() const noexcept;
  
  std::shared_ptr<concurrency::executor::ThreadPool>
  background_executor() const noexcept;

  std::shared_ptr<concurrency::executor::Thread>
  thread_executor() const noexcept;

  std::shared_ptr<concurrency::executor::WorkerThread>
  make_worker_thread_executor();

  std::shared_ptr<concurrency::executor::Manual>
  make_manual_executor();

 public:
  static std::tuple<uint32_t, uint32_t, uint32_t> version() noexcept;

 public:
   void add(std::shared_ptr<net::connection::Manager> net);
   void remove(std::shared_ptr<net::connection::Manager> net) noexcept;
   void remove_net(std::string_view name) noexcept;

 public:
   bool enable_console(std::string_view addr = {}) noexcept;
   void register_console_handler(
     std::string_view cmd, console_func func,
     std::string_view desc = {}) noexcept;

 public:
  template<class F, class... Args>
  auto enqueue(F&& f, Args&&... args) 
  -> std::future<typename std::invoke_result_t<F, Args...>> {
    using return_type = typename std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (!running_)
         throw std::runtime_error("enqueue on stopped Kernel");
      tasks_.emplace([task](){ (*task)(); });
    }
   return res;
  }

  template<class F, class... Args>
  std::thread::id newthread(
      const std::string &name, F&& f, Args&&... args) {
    using return_type = typename std::invoke_result_t<F, Args...>;
    std::thread::id res;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (!running_)
         throw std::runtime_error("newthread on stopped Kernel");
      auto task = std::make_shared< std::packaged_task<return_type()> >(
          std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
      thread_workers_.emplace_back([task, name](){ 
        thread::start();
        ThreadCollect tc;
        std::future<return_type> task_res = task->get_future();
        for (;;) {
          if (thread::is_stopping()) break;
          (*task)(); 
          (*task).reset(); //Remeber it, the packaged_task reset then can call again.
          using is_bool_result = std::is_same<return_type, bool>;
          thread::check_running(is_bool_result{}, task_res);
        }

        //Log(this log also can enable with app.debug).
        auto id_str = thread::get_id();
        LOG_DEBUG << name << " stopping with " << id_str;
      });
      res = thread_workers_[thread_workers_.size() - 1].get_id();
    }

    //Log(this log also can enable with app.debug).
    std::stringstream id_str;
    id_str << res;
    LOG_DEBUG << name << " starting with " << id_str.str();
    return res;
  }

 private:
  bool running_;
  std::mutex mutex_;
  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> thread_workers_;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace plain

#ifndef ENGINE
#define ENGINE plain::Singleton<plain::Kernel>::get_instance()
#endif

#endif // PLAIN_ENGINE_KERNEL_H_
