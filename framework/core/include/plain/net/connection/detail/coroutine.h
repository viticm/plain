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

using await_func =
  std::function<bool(concurrency::coroutine_handle<Task::promise_type>)>;

class AwaitableBasic : public concurrency::suspend_always {

 public:
  AwaitableBasic(await_func func) noexcept {}
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
    return static_cast<bool>(await_) && await_(handle);
  }

};


} // namespace connection::detail
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_DETAIL_COROUTINE_H_
