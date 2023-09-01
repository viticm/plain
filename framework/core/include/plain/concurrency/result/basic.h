/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/24 17:26
 * @uses The concurrency result basic(template implemention).
 */

#ifndef PLAIN_CONCURRENCY_RESULT_BASIC_H_
#define PLAIN_CONCURRENCY_RESULT_BASIC_H_

#include "plain/concurrency/result/config.h"
#include "plain/basic/bind.h"
#include "plain/basic/noncopyable.h"
#include "plain/basic/logger.h"
#include "plain/concurrency/result/awaitable.h"
#include "plain/concurrency/result/detail/state.h"

namespace plain::concurrency {

template <typename T>
requires is_result_type<T>
class Result : noncopyable {

  friend class result::detail::when_helper;
  friend struct result::detail::shared_helper;

 public:
  Result() noexcept = default;
  Result(Result &&) noexcept = default;
  Result(result::detail::consumer_state_ptr_t<T> state) noexcept :
    state_(std::move(state)) {}
  Result(result::detail::State<T> *state) noexcept : state_(state) {}

 public:
  explicit operator bool() const noexcept {
    return static_cast<bool> (state_);
  }

 public:
  ResultStatus status() const noexcept {
    assert(static_cast<bool> (state_));
    return state_->status();
  }
  
  void wait() const noexcept {
    assert(static_cast<bool> (state_));
    state_->wait();
  }

  template <typename Rep, typename Period = std::ratio<1>>
  ResultStatus wait_for(std::chrono::duration<Rep, Period> duration) noexcept {
    assert(static_cast<bool> (state_));
    return state_->wait_for(duration);
  }

  template <typename Clock, typename Duration = typename Clock::duration>
  ResultStatus wait_until(
    const std::chrono::time_point<Clock, Duration>& timeout_time) noexcept {
    assert(static_cast<bool> (state_));
    return state_->wait_until(timeout_time);
  }

  T get() const noexcept {
    assert(static_cast<bool> (state_));
    state_->wait();
    T r{};
    try {
      result::detail::joined_consumer_state_ptr_t<T> state(state_->release());
      r = state->get();
    } catch (const std::exception &e) {
      LOG_ERROR << "Result::get error: " << e.what();    
    }
    return r;
  }

 private:
  result::detail::consumer_state_ptr_t<T> state_;

};

namespace result {

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_BASIC_H_
