/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id lazy_awaitable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/07 18:07
 * @uses The concurrency lazy result awaitable.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_LAZY_AWAITABLE_H_
#define PLAIN_CONCURRENCY_RESULT_LAZY_AWAITABLE_H_

#include "plain/concurrency/result/config.h"
#include <cassert>
#include <utility>
#include "plain/basic/logger.h"
#include "plain/concurrency/result/detail/lazy_state.h"

namespace plain::concurrency {
namespace result {

template <typename T>
class LazyAwaitable {

 public:
  LazyAwaitable(coroutine_handle<detail::LazyState<T>> state) noexcept
    : state_{state} {
    assert(static_cast<bool>(state_));
  }

  LazyAwaitable(const LazyAwaitable &) = delete;
  LazyAwaitable(LazyAwaitable &&) = delete;
  ~LazyAwaitable() {
    auto state = state_;
    state.destroy();
  }

 public:
  bool await_ready() const noexcept {
    return state_.done();
  }

  coroutine_handle<void> await_suspend(coroutine_handle<void> handle) noexcept {
    return state_.promise().await(handle);
  }

  T await_resume() noexcept {
    T r{};
    try {
      r = state_.promise().get();
    } catch(const std::exception &e) {
      LOG_ERROR << "LazyAwaitable::await_resume exception: " << e.what();
    }
    return r;
  }

 private:
  coroutine_handle<detail::LazyState<T>> state_;

};

template <typename T>
class LazyResolveAwaitable {

 public:
  LazyResolveAwaitable(coroutine_handle<detail::LazyState<T>> state) noexcept
    : state_{state} {
    assert(static_cast<bool>(state_));
  }

  LazyResolveAwaitable(const LazyResolveAwaitable &) = delete;
  LazyResolveAwaitable(LazyResolveAwaitable &&) = delete;
  ~LazyResolveAwaitable() {
    if (static_cast<bool>(state_))
      state_.destroy();
  }

 public:
  bool await_ready() const noexcept {
    return state_.done();
  }

  coroutine_handle<void> await_suspend(coroutine_handle<void> handle) noexcept {
    return state_.promise().await(handle);
  }

  LazyResult<T> await_resume() noexcept {
    return {std::exchange(state_, {})};
  }

 private:
  coroutine_handle<detail::LazyState<T>> state_;

};


} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_LAZY_AWAITABLE_H_
