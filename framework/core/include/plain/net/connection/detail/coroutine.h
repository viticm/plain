/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id coroutine.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/12/23 17:06
 * @uses The net connection coroutine implemention.
 */

#ifndef PLAIN_NET_CONNECTION_DETAIL_COROUTINE_H_
#define PLAIN_NET_CONNECTION_DETAIL_COROUTINE_H_

#include "plain/net/connection/detail/config.h"
#include "plain/sys/thread.h"
#include "plain/concurrency/config.h"

namespace plain::net {
namespace connection::detail {
  
struct Task {
  struct promise_type {
    promise_type() {
    
    }
    
    Task get_return_object() {
      return {std::coroutine_handle<Task::promise_type>::from_promise(*this)};
    }
    
    std::suspend_never initial_suspend() {
      return {};
    }
    
    std::suspend_never final_suspend() noexcept {
      return {};
    }
    
    void return_void() {

    }
    
    void unhandled_exception() {}
  };

  concurrency::coroutine_handle<Task::promise_type> handle_;
};

using await_func = std::function<void()>;

class AwaitableBasic : public concurrency::suspend_always {

 public:
  AwaitableBasic(await_func func) noexcept : await_{func} {}
  AwaitableBasic(const AwaitableBasic &) = delete;
  AwaitableBasic(AwaitableBasic &&) = delete;

 protected:
  await_func await_;

};

class Awaitable : public AwaitableBasic {

 public:
  Awaitable(await_func func) noexcept : AwaitableBasic(func) {
  }

 public:
  bool await_suspend(
    concurrency::coroutine_handle<Task::promise_type> handle) noexcept {
    if (static_cast<bool>(await_)) {
      // std::future<bool> r = std::async(std::launch::async, await_, handle);
      // return r.get();
      thread_t([await = await_, handle] { await(); handle(); }).detach();
      return true;
    }
    return false;
  }

};


} // namespace connection::detail
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_DETAIL_COROUTINE_H_
