/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id kernel.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/20 11:39
 * @uses your description
*/
#ifndef PF_ENGINE_KERNEL_TCC_
#define PF_ENGINE_KERNEL_TCC_

#include "pf/sys/thread.h"
#include "pf/basic/util.h"
#include "pf/basic/time_manager.h"
#include "pf/engine/kernel.h"

namespace pf_engine {

inline void worksleep(uint32_t starttime) {
  auto worktime = 
    static_cast< int32_t >(TIME_MANAGER_POINTER->get_tickcount() - starttime);
  auto time = static_cast<int32_t>(
      1000 / GLOBALS["default.engine.frame"].get<int32_t>()) - worktime;
  if (time > 0) std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

template<class F, class... Args>
auto Kernel::enqueue(F&& f, Args&&... args) 
-> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;
  auto task = std::make_shared< std::packaged_task<return_type()> >(
  std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );
  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_)
       throw std::runtime_error("enqueue on stopped Kernel");
    tasks_.emplace([task](){ (*task)(); });
  }
 return res;
}

template<class F, class... Args>
std::thread::id Kernel::newthread(F&& f, Args&&... args) {
  using return_type = typename std::result_of<F(Args...)>::type;
  std::thread::id res;
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_)
       throw std::runtime_error("newthread on stopped Kernel");
    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );
    thread_workers_.emplace_back([task](){ 
      pf_sys::thread::start();
      pf_sys::ThreadCollect tc;
      std::future<return_type> task_res = task->get_future();
      for (;;) {
        if (pf_sys::thread::is_stopping()) break;
        auto starttime = TIME_MANAGER_POINTER->get_tickcount();
        (*task)(); 
        if (std::is_same<decltype(task_res), bool>::value && !task_res.get())
          pf_sys::thread::stop();
        worksleep(starttime);
        (*task).reset(); //Remeber it, the packaged_task reset then can call again.
      }
    });
    res = thread_workers_[thread_workers_.size() - 1].get_id();
  }
  return res;
}

}; //namespace pf_engine

#endif //PF_ENGINE_KERNEL_TCC_
