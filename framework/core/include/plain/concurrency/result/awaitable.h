/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id awaitable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/07 15:07
 * @uses The concurrency result awaitable.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_AWAITABLE_H_
#define PLAIN_CONCURRENCY_RESULT_AWAITABLE_H_

#include "plain/concurrency/result/config.h"
#include "plain/basic/logger.h"
#include "plain/concurrency/result/detail/state.h"

namespace plain::concurrency {
namespace result {

namespace detail {

template <typename T>
class AwaitableBasic : public suspend_always {

 public:
  AwaitableBasic(consumer_state_ptr_t<T> state) noexcept :
    state_{std::move(state)} {}
  AwaitableBasic(const AwaitableBasic &) = delete;
  AwaitableBasic(AwaitableBasic &&) = delete;

 protected:
  consumer_state_ptr_t<T> state_;

};

} // namespace detail

template <typename T>
class Awaitable : public detail::AwaitableBasic<T> {

 public:
  Awaitable(detail::consumer_state_ptr_t<T> state) noexcept
    : detail::AwaitableBasic<T>(std::move(state)) {
  }

 public:
  bool await_suspend(coroutine_handle<void> handle) noexcept {
    assert(static_cast<bool>(this->state_));
    return this->state_->await(handle);
  }

  T await_resume() {
    detail::joined_consumer_state_ptr_t<T> state(this->state_.release());
    return state->get();
  }

};

template <typename T>
class ResolveAwaitable : public detail::AwaitableBasic<T> {
 
 public:   
  ResolveAwaitable(detail::consumer_state_ptr_t<T> state) noexcept
    : detail::AwaitableBasic<T>(std::move(state)) {
  }

 public:
  bool await_suspend(coroutine_handle<void> handle) noexcept {
    assert(static_cast<bool>(this->state_));
    return this->state_->await(handle);
  }

  Result<T> await_resume() noexcept {
    return Result<T>(std::move(this->state_));
  }

};

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_AWAITABLE_H_
