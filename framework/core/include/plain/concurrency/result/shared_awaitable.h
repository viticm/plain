/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id shared_awaitable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/08 16:36
 * @uses The concurrency result shared awaitable.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_SHARED_AWAITABLE_H_
#define PLAIN_CONCURRENCY_RESULT_SHARED_AWAITABLE_H_

#include "plain/concurrency/result/config.h"
#include "plain/concurrency/result/detail/shared_state.h"

namespace plain::concurrency {
namespace result {

namespace detail {

template <typename T>
class SharedAwaitableBasic : public suspend_always {

 public:
  SharedAwaitableBasic(const std::shared_ptr<SharedState<T>> &state)
    : state_{state} {
  }
  SharedAwaitableBasic(const SharedAwaitableBasic &) = delete;
  SharedAwaitableBasic(SharedAwaitableBasic &&) = delete;

 protected:
  std::shared_ptr<SharedState<T>> state_;

};

} // namespace detail

template <typename T>
class SharedAwaitable : public detail::SharedAwaitableBasic<T> {

 public:
  SharedAwaitable(
    const std::shared_ptr<detail::SharedState<T>> &state) noexcept
      : detail::SharedAwaitableBasic<T>(state) {
  }

  bool await_suspend(coroutine_handle<void> handle) noexcept {
    assert(static_cast<bool>(this->state_));
    await_context_.callable = handle;
    return this->state_->await(await_context_);
  }

  std::add_lvalue_reference_t<T> await_resume() {
    return this->state_->get();
  }

 private:
   detail::SharedAwaitContext await_context_;

};

template <typename T>
class SharedResolveAwaitable : public detail::SharedAwaitableBasic<T> {

 public:
  SharedResolveAwaitable(
    const std::shared_ptr<detail::SharedState<T>> &state) noexcept
      : detail::SharedAwaitableBasic<T>(state) {
  }

  bool await_suspend(coroutine_handle<void> handle) noexcept {
    assert(static_cast<bool>(this->state_));
    await_context_.callable = handle;
    return this->state_->await(await_context_);
  }

  Shared<T> await_resume() {
    return Shared<T>(std::move(this->state_));
  }

 private:
   detail::SharedAwaitContext await_context_;

};

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_SHARED_AWAITABLE_H_
