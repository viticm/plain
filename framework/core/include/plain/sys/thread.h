/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id thread.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/07 19:22
 * @uses The thread module.
 *       Pool refer: https://github.com/progschj/ThreadPool.
*/
#ifndef PLAIN_SYS_THREAD_H_
#define PLAIN_SYS_THREAD_H_

#include "plain/sys/config.h"
#include <thread>
#include "plain/basic/io.h"
#include "plain/basic/logger.h"
#include "plain/basic/global.h"

namespace plain {

class PLAIN_API ThreadPool {
 public:
  ThreadPool(size_t);
  template<class F, class... Args>
  auto enqueue(F&& f, Args&&... args) 
  -> std::future<typename std::result_of<F(Args...)>::type>;
  ~ThreadPool();
 private:
  // need to keep track of threads so we can join them
  std::vector<std::thread> workers_;
  // the task queue
  std::queue<std::function<void()>> tasks_;
  // synchronization
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  bool stop_;
};
 

// 用于需要回收的线程数量统计，只要线程任务没有完成，主线程就会等待
class PLAIN_API ThreadCollect {

 public:
  ThreadCollect();
  ~ThreadCollect();

 public:
  static int32_t count() {
    return count_;
  }

 private:
  static std::atomic<int32_t> count_;

};

namespace thread {

inline const std::string get_id() {
  std::stringstream ss;
  ss << std::this_thread::get_id();
  return std::string(ss.str());
}

inline const std::string get_id(const std::thread& thread) {
  std::stringstream ss;
  ss << thread.get_id();
  return std::string(ss.str());
}

//用下面的方法的线程可以启动与停止
inline const std::string status_key(std::thread& thread) {
  std::string _status_key{"thread.status."};
  _status_key += get_id(thread);
  return _status_key;
}

inline const std::string status_key() {
  std::string _status_key{"thread.status."};
  _status_key += get_id();
  return _status_key;
}

template<typename _Callable, typename... _Args>
inline void start(std::thread& thread, _Callable &&__f, _Args...__args) {
  thread = std::move(std::thread(
        std::forward<_Callable>(__f), 
        std::forward<_Args>(__args)...));
  const std::string _status_key = status_key(thread);
  GLOBALS[_status_key] = kThreadStatusRunning;
}

inline void start(std::thread& thread) {
  const std::string _status_key = status_key(thread);
  GLOBALS[_status_key] = kThreadStatusRunning;
}

inline void start() {
  const std::string _status_key = status_key();
  GLOBALS[_status_key] = kThreadStatusRunning;
}

inline void stop() {
  const std::string _status_key = status_key();
  GLOBALS[_status_key] = kThreadStatusStopped;
}

inline void stop(std::thread& thread) {
  const std::string _status_key = status_key(thread);
  if (GLOBALS[_status_key] == kThreadStatusRunning) 
    GLOBALS[_status_key] = kThreadStatusStopped;
}

inline uint8_t status(std::thread& thread) {
  const std::string _status_key = status_key(thread);
  return GLOBALS[_status_key].get<uint8_t>();
}

inline bool is_running(std::thread& thread) {
  return kThreadStatusRunning == status(thread);
}

inline bool is_stopping(std::thread& thread) {
  return kThreadStatusStopped == status(thread);
}

inline uint8_t status() {
  const std::string _status_key = status_key();
  return GLOBALS[_status_key].get<uint8_t>();
}

inline bool is_running() {
  return kThreadStatusRunning == status();
}

inline bool is_stopping() {
  return kThreadStatusStopped == status();
}

// FIXME: optimize the Kernel::newthread and this.
template <class F, class... Args>
std::thread create(const std::string_view& name, F&& f, Args&&... args) {
  using return_type = typename std::result_of<F(Args...)>::type;
  auto task = std::make_shared< std::packaged_task<return_type()> >(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );
  auto r = std::thread([task, name](){ 
    start();
    ThreadCollect tc;
    std::future<return_type> task_res = task->get_future();
    for (;;) {
      if (is_stopping()) break;
      (*task)(); 
      (*task).reset(); //Remeber it, the packaged_task reset then can call again.
      // FIXME: std::is_same not work.
      if (std::is_same<return_type, bool>::value && !task_res.get())
        thread::stop();
    }

    if (true == GLOBALS["app.debug"]) {
      auto id_str = get_id();
      LOG_DEBUG << name.data() << " stopping with " << id_str;
    }
  });

  // Log(this log also can enable with app.debug).

  if (true == GLOBALS["app.debug"]) {
    std::stringstream id_str;
    id_str << r.get_id();
    LOG_DEBUG << name.data() << " starting with " << id_str.str();
  }
  return r;
}

} //namespace thread

inline ThreadCollect::ThreadCollect() {
  ++count_;
}

inline ThreadCollect::~ThreadCollect() {
  --count_;
  auto count = count_.load(std::memory_order_relaxed);
  if (true == GLOBALS["app.debug"]) {
    plain::io_cdebug("[%s] thread(%s) collect wait exit: %d", 
                     GLOBALS["app.name"].c_str(), 
                     thread::get_id().c_str(),
                     count);
  }
}

} //namespace plain

#endif //PLAIN_SYS_THREAD_H_
