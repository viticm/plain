/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id lazy_state.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:15
 * @uses The concurrency result lazy state detail.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_LAZY_STATE_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_LAZY_STATE_H_

#include "plain/concurrency/result/detail/config.h"

namespace plain::concurrency {
namespace result::detail {

struct LazyFinalAwaiter : public suspend_always {
  template <class T>
  coroutine_handle<void> await_suspend(coroutine_handle<T> handle) noexcept {
    return handle.promise().resume_caller();
  }
};

class LazyStateBasic {

 public:
  coroutine_handle<void> resume_caller() const noexcept {
    return coroutine_handle_;
  }

  coroutine_handle<void> await(coroutine_handle<void> handle) noexcept {
    coroutine_handle_ = handle;
    return coroutine_handle<LazyStateBasic>::from_promise(*this);
  }

 protected:
  coroutine_handle<void> coroutine_handle_;

};

template <typename T>
class LazyState {

 public:
  ResultStatus status() const noexcept {
    return producer_.status();
  }

  LazyResult<T> get_return_object() noexcept {
    const auto self_handle = coroutine_handle<LazyState>::from_promise(*this);
    return LazyResult<T>(self_handle);
  }

  void unhandled_exception() noexcept {
    producer_.build_exception(std::current_exception());
  }

  suspend_always initial_suspend() const noexcept {
    return {};
  }

  LazyFinalAwaiter final_suspend() const noexcept {
    return {};
  }

  template<class... Args>
  void set_result(Args&&... args)
    noexcept(noexcept(T(std::forward<Args>(args)...))) {
    producer_.build_result(std::forward<Args>(args)...);
  }
  
  T get() {
    return producer_.get();
  }

 private:
  ProducerContext<T> producer_;

};

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_LAZY_STATE_H_
