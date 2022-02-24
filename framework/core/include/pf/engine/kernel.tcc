/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id kernel.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/20 11:39
 * @uses The engine kernel template.
*/
#ifndef PF_ENGINE_KERNEL_TCC_
#define PF_ENGINE_KERNEL_TCC_

#include "pf/sys/thread.h"
#include "pf/basic/util.h"
#include "pf/basic/logger.h"
#include "pf/basic/time_manager.h"
#include "pf/engine/kernel.h"

namespace pf_engine {

// For application can use framework without kernel.init
inline bool init_basic_env() {
  using namespace pf_basic;

  //Time manager.
  if (is_null(TIME_MANAGER_POINTER)) {
    auto time_manager = new TimeManager();
    if (is_null(time_manager)) return false;
    unique_move(TimeManager, time_manager, g_time_manager);
    if (!g_time_manager->init()) return false;
  }

  //Logger.
  if (is_null(LOGSYSTEM_POINTER)) {
    auto logger = new Logger();
    if (is_null(logger)) return false;
    unique_move(Logger, logger, g_logger);
  }
  return true;
}

inline void worksleep(uint64_t starttime, const std::string &name) {
  auto worktime = 
    static_cast< int32_t >(TIME_MANAGER_POINTER->get_tickcount() - starttime);
  static int32_t frame_time = 
    GLOBALS["default.engine.frame_time"].get<int32_t>();
  auto time = frame_time - worktime;
  if (time > 1000) {
    SLOW_ERRORLOG(ENGINE_MODULENAME,
                  "[%s] worksleep(%s) time error %d|%d|%d|%d",
                  ENGINE_MODULENAME,
                  name.c_str(),
                  time,
                  starttime,
                  worktime,
                  frame_time);
    time = 10;
  }
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
std::thread::id Kernel::newthread(
    const std::string &name, F&& f, Args&&... args) {
  using return_type = typename std::result_of<F(Args...)>::type;
  std::thread::id res;
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_)
       throw std::runtime_error("newthread on stopped Kernel");
    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );
    thread_workers_.emplace_back([task, name](){ 
      pf_sys::thread::start();
      pf_sys::ThreadCollect tc;
      std::future<return_type> task_res = task->get_future();
#if _DEBUG
      auto lasttime = TIME_MANAGER_POINTER->get_tickcount();
#endif
      for (;;) {
        if (pf_sys::thread::is_stopping()) break;
        auto starttime = TIME_MANAGER_POINTER->get_tickcount();
#if _DEBUG
        auto diff_time = starttime - lasttime;
        if (diff_time > 1000) {
          // std::cout << "thread: " << name << " working" << std::endl;
          SLOW_DEBUGLOG(ENGINE_MODULENAME,
                        "[%s] thread(%s) time work start: %d",
                        ENGINE_MODULENAME,
                        name.c_str(),
                        starttime);

          lasttime = starttime;
        }
#endif
        (*task)(); 
        (*task).reset(); //Remeber it, the packaged_task reset then can call again.
        if (std::is_same<decltype(task_res), bool>::value && !task_res.get())
          pf_sys::thread::stop();
        worksleep(starttime, name);
      }

      //Log(this log also can enable with app.debug).
      auto id_str = pf_sys::thread::get_id();
      SLOW_DEBUGLOG(ENGINE_MODULENAME,
                    "[%s] Kernel::newthread(%s) stop with %s.",
                    ENGINE_MODULENAME,
                    name.c_str(),
                    id_str.c_str());
    });
    res = thread_workers_[thread_workers_.size() - 1].get_id();
  }

  //Log(this log also can enable with app.debug).
  std::stringstream id_str;
  id_str << res;
  SLOW_DEBUGLOG(ENGINE_MODULENAME,
                "[%s] Kernel::newthread(%s) start with %s.",
                ENGINE_MODULENAME,
                name.c_str(),
                id_str.str().c_str());
  return res;
}

} //namespace pf_engine

#endif //PF_ENGINE_KERNEL_TCC_
