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
#include "plain/sys/constants.h"

namespace plain {

using thread_t = std::jthread;

class PLAIN_API ThreadPool {
 public:
  ThreadPool(size_t);
  template<typename F, typename... Args>
  auto enqueue(F&& f, Args&&... args) 
  -> std::future<typename std::invoke_result_t<F, Args...>>;
  ~ThreadPool();
 private:
  // need to keep track of threads so we can join them
  std::vector<thread_t> workers_;
  // the task queue
  std::queue<std::function<void()>> tasks_;
  // synchronization
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  bool stop_;
};
 
// A tiny thread watch cllect count implemention.
class PLAIN_API ThreadCollect {

 public:
  ThreadCollect();
  ~ThreadCollect();

 public:
  static int32_t count() {
    return count_.load(std::memory_order_relaxed);
  }

 private:
  static std::atomic_int32_t count_;

};

namespace thread {

void set_name(const std::string_view &name) noexcept;

std::uintptr_t get_current_virtual_id() noexcept;

inline const std::string get_id() {
  std::stringstream ss;
  ss << std::this_thread::get_id();
  return std::string(ss.str());
}

inline const std::string get_id(const thread_t& thread) {
  std::stringstream ss;
  ss << thread.get_id();
  return std::string(ss.str());
}

//用下面的方法的线程可以启动与停止
inline const std::string status_key(thread_t& thread) {
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
inline void start(thread_t& thread, _Callable &&__f, _Args...__args) {
  thread = std::move(thread_t(
        std::forward<_Callable>(__f), std::forward<_Args>(__args)...));
  const std::string _status_key = status_key(thread);
  GLOBALS[_status_key] = ThreadStatus::Running;
}

inline void start(thread_t& thread) {
  const std::string _status_key = status_key(thread);
  GLOBALS[_status_key] = ThreadStatus::Running;
}

inline void start() {
  const std::string _status_key = status_key();
  GLOBALS[_status_key] = ThreadStatus::Running;
}

inline void stop() {
  const std::string _status_key = status_key();
  GLOBALS[_status_key] = ThreadStatus::Stopped;
}

inline void stop(thread_t& thread) {
  const std::string _status_key = status_key(thread);
  if (GLOBALS[_status_key] == ThreadStatus::Running) 
    GLOBALS[_status_key] = ThreadStatus::Stopped;
}

inline variable_t status(thread_t& thread) {
  const std::string _status_key = status_key(thread);
  return GLOBALS[_status_key];
}

inline bool is_running(thread_t& thread) {
  return ThreadStatus::Running == status(thread);
}

inline bool is_stopping(thread_t& thread) {
  return ThreadStatus::Stopped == status(thread);
}

inline variable_t status() {
  const std::string _status_key = status_key();
  return GLOBALS[_status_key];
}

inline bool is_running() {
  return ThreadStatus::Running == status();
}

inline bool is_stopping() {
  return ThreadStatus::Stopped == status();
}

template <typename T>
void check_running(std::true_type, std::future<T> &task_res) {
  if (!task_res.get()) stop();
}

template <typename T>
void check_running(std::false_type, std::future<T> &task_res) {
  stop();
}

// FIXME: optimize the Kernel::newthread and this.
// With endless loop excute F(F return false exit).
template <typename F, typename... Args>
// requires std::predicate<F, Args...>
thread_t create(const std::string_view &name, F&& f, Args&&... args) {
  using return_type = typename std::invoke_result_t<F, Args...>;
  auto task = std::make_shared< std::packaged_task<return_type()> >(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );
  auto r = thread_t([task, name](){ 
    set_name(name);
    start();
    ThreadCollect tc;
    std::future<return_type> task_res = task->get_future();
    for (;;) {
      if (is_stopping()) break;
      (*task)(); 
      (*task).reset(); //Remeber it, the packaged_task reset then can call again.
      using is_bool_result = std::is_same<bool, return_type>;
      check_running(is_bool_result{}, task_res);
    }

    if (true == GLOBALS["app.debug"]) {
      auto id_str = get_id();
      LOG_DEBUG << name.data() << " stopping with " << id_str;
    }
  });

  if (true == GLOBALS["app.debug"]) {
    std::stringstream id_str;
    id_str << r.get_id();
    LOG_DEBUG << name.data() << " starting with " << id_str.str();
  }
  return r;
}

inline size_t hardware_concurrency() noexcept {
  const auto hc = std::thread::hardware_concurrency();
  return hc != 0 ? hc : kSystemDefaultNumberOfCores;
}

} //namespace thread

inline ThreadCollect::ThreadCollect() {
  count_.fetch_add(1, std::memory_order_relaxed);
}

inline ThreadCollect::~ThreadCollect() {
  count_.fetch_sub(1, std::memory_order_relaxed);
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
