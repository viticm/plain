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
  Shared() = default;
  ~Shared() = default;

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

  Shared &operator=(Shared &&rhs) noexcept {
    if (this != &rhs && state_ != rhs.state_)
      state_ = std::move(rhs.state_);
    return *this;
  }

  Shared(const Shared &) noexcept = default;
  Shared(Shared &&) noexcept = default;

  operator bool() const noexcept {
    return static_cast<bool>(state_.get());
  }

  auto operator co_await() {
    throw_if_empty("co_await - result is empty.");
    return SharedAwaitable<T>(state_);
  }

 public:
  ResultStatus status() const {
    throw_if_empty("status - result is empty.");
    return state_->status();
  }

  void wait() {
    throw_if_empty("wait - result is empty.");
    state_->wait();
  }

  template <typename Rep, typename Period = std::ratio<1>>
  ResultStatus wait_for(std::chrono::duration<Rep, Period> duration) {
    throw_if_empty("wait_for - result is empty.");
    return state_->wait_for(duration);
  }
  template <typename Clock, typename Duration = typename Clock::duration>
  ResultStatus wait_until(
      const std::chrono::time_point<Clock, Duration>& timeout_time) {
    throw_if_empty("wait_until - result is empty.");
    return state_->wait_until(timeout_time);
  }

  std::add_lvalue_reference_t<T> get() {
    throw_if_empty("get - result is empty.");
    state_->wait();
    return state_->get();
  }

  auto resolve() {
    throw_if_empty("resolve - result is empty.");
    return SharedResolveAwaitable<T>(state_);
  }

 private:
  std::shared_ptr<detail::SharedState<T>> state_;

 private:
  void throw_if_empty(const char *error_msg) const {
    if (!static_cast<bool>(state_))
      throw std::runtime_error(error_msg);
  }

};

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_SHARED_H_
