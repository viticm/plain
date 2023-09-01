/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id shared.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/08 09:50
 * @uses The concurrency shared result.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_SHARED_H_
#define PLAIN_CONCURRENCY_RESULT_SHARED_H_

#include "plain/concurrency/result/config.h"
#include "plain/basic/copyable.h"
#include "plain/concurrency/result/shared_awaitable.h"
#include "plain/concurrency/result/detail/shared_state.h"

namespace plain::concurrency {
namespace result {

template <typename T>
class Shared : copyable {

 public:
  Shared(std::shared_ptr<detail::SharedState<T>> state) noexcept
    : state_{std::move(state)} {
  }

  Shared(Result<T> rhs) {
    if (!static_cast<bool>(rhs)) return;
    auto state = detail::shared_helper::get_state(rhs);
    state_ = std::make_shared<detail::SharedState<T>>(std::move(state));
    state_->share(std::static_pointer_cast<detail::SharedStateBasic>(state_));
  }

  Shared &operator=(const Shared &rhs) noexcept {
    if (this != &rhs && state_ != rhs.state_)
      state_ = rhs.state_;
    return *this;
  }

  operator bool() const noexcept {
    return static_cast<bool>(state_.get());
  }

  auto operator co_await() noexcept {
    assert(static_cast<bool>(state_));
    return SharedAwaitable<T>(state_);
  }

 public:
  ResultStatus status() const noexcept {
    assert(static_cast<bool>(state_));
    return state_->status();
  }

  void wait() noexcept {
    assert(static_cast<bool>(state_));
    state_->wait();
  }

  template <typename Rep, typename Period = std::ratio<1>>
  ResultStatus wait_for(std::chrono::duration<Rep, Period> duration) {
    assert(static_cast<bool>(state_));
    return state_->wait_for(duration);
  }
  template <typename Clock, typename Duration = typename Clock::duration>
  ResultStatus wait_until(
      const std::chrono::time_point<Clock, Duration>& timeout_time) {
    assert(static_cast<bool>(state_));
    return state_->wait_until(timeout_time);
  }

  std::add_lvalue_reference_t<T> get() {
    assert(static_cast<bool>(state_));
    state_->wait();
    return state_->get();
  }

  auto resolve() noexcept {
    assert(static_cast<bool>(state_));
    return SharedResolveAwaitable<T>(state_);
  }

 private:
  std::shared_ptr<detail::SharedState<T>> state_;

};

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_SHARED_H_
