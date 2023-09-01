/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id lazy.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/07 17:28
 * @uses The concurrency lazy result.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_LAZY_H_
#define PLAIN_CONCURRENCY_RESULT_LAZY_H_

#include "plain/concurrency/result/config.h"
#include <cassert>
#include <utility>
#include "plain/concurrency/result/detail/lazy_state.h"
#include "plain/concurrency/result/lazy_awaitable.h"

namespace plain::concurrency {

template <typename T>
class LazyResult {

 public:
  LazyResult() noexcept = default;
  LazyResult(LazyResult &&rhs) noexcept
    : state_{std::exchange(rhs.state_, {})} {

  }
  LazyResult(coroutine_handle<result::detail::LazyState<T>> state) noexcept
    : state_{state} {

  }
  ~LazyResult() noexcept {
    if (static_cast<bool>(state_))
      state_.destroy();
  }
  LazyResult &operator=(LazyResult &&rhs) {
    if (&rhs == this) return *this;
    if (static_cast<bool>(state_))
      state_.destroy();
    state_ = std::exchange(rhs.state_, {});
    return *this;
  }

 public:
  explicit operator bool() const noexcept {
    return static_cast<bool>(state_);
  }

  auto operator co_await() noexcept {
    return LazyResult<T>{std::exchange(state_, {})};
  }

 public:
  ResultStatus status() const noexcept {
    assert(static_cast<bool>(state_));
    return state_.promise().status();
  }

  auto resolve() noexcept {
    assert(static_cast<bool>(state_));
    return result::LazyResolveAwaitable<T>{std::exchange(state_, {})};
  }

  Result<T> run() noexcept {
    assert(static_cast<bool>(state_));
    return run_impl();
  }

 private:
  Result<T> run_impl() {
    LazyResult self(std::move(*this));
    co_return co_await self;
  }

 private:
  coroutine_handle<result::detail::LazyState<T>> state_;

};

} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_LAZY_H_
