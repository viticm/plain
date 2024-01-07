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

  friend class result::detail::WhenHelper;
  friend struct result::detail::shared_helper;

 public:
  Result() noexcept = default;
  Result(Result &&) noexcept = default;
  Result(result::detail::consumer_state_ptr_t<T> state) noexcept :
    state_(std::move(state)) {
  }
  Result(result::detail::State<T> *state) noexcept : state_(state) {
  }

 public:
  explicit operator bool() const noexcept {
    return static_cast<bool> (state_);
  }

  Result &operator=(Result &&rhs) noexcept {
    if (this != &rhs)
      state_ = std::move(rhs.state_);
    return *this;
  }
  
  auto operator co_await() {
    throw_if_empty("co_await - result is empty.");
    return result::Awaitable<T>{std::move(state_)};
  }

 public:
  ResultStatus status() const {/*noexcept {*/
    throw_if_empty("status - result is empty.");
    return state_->status();
  }
  
  void wait() const {//noexcept {
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

  T get() {
    throw_if_empty("get - result is empty.");
    state_->wait();
    result::detail::joined_consumer_state_ptr_t<T> state(state_.release());
    return state->get();
  }

  auto resolve() {
    throw_if_empty("resolve - result is empty.");
    return result::ResolveAwaitable{std::move(state_)};
  }

 private:
  result::detail::consumer_state_ptr_t<T> state_;

 private:
  void throw_if_empty(const char *error_msg) const {
    if (!static_cast<bool>(state_))
      throw std::runtime_error(error_msg);
  }

};

template <typename T>
class ResultPromise {

  static constexpr auto valid_result_type_v =
    std::is_same_v<T, void> || std::is_nothrow_move_constructible_v<T>;

  static_assert(
    valid_result_type_v,
    "ResultPromise<T> - <<T>> should be no-throw-move constructible or void.");

 private:
  result::detail::producer_state_ptr_t<T> producer_state_;
  result::detail::consumer_state_ptr_t<T> consumer_state_;

  void throw_if_empty(const char* message) const {
    if (!static_cast<bool>(producer_state_)) {
      throw std::runtime_error(message);
    }
  }

  void break_task_if_needed() noexcept {
    if (!static_cast<bool>(producer_state_)) {
      return;
    }

    if (static_cast<bool>(consumer_state_)) {  // no result to break.
      return;
    }

    auto exception_ptr = std::make_exception_ptr(
      std::runtime_error("associated task was interrupted abnormally"));
    producer_state_->set_exception(exception_ptr);
    producer_state_.reset();
  }

 public:
  ResultPromise() {
    producer_state_.reset(new result::detail::State<T>());
    consumer_state_.reset(producer_state_.get());
  }

  ResultPromise(ResultPromise &&rhs) noexcept :
    producer_state_(std::move(rhs.producer_state_)),
    consumer_state_(std::move(rhs.consumer_state_)) {

  }

  ~ResultPromise() noexcept {
    break_task_if_needed();
  }

  ResultPromise &operator=(ResultPromise &&rhs) noexcept {
    if (this != &rhs) {
      break_task_if_needed();
      producer_state_ = std::move(rhs.producer_state_);
      consumer_state_ = std::move(rhs.consumer_state_);
    }

    return *this;
  }

  ResultPromise(const ResultPromise &) = delete;
  ResultPromise &operator=(const ResultPromise &) = delete;

  explicit operator bool() const noexcept {
    return static_cast<bool>(producer_state_);
  }

  template <typename ...Args>
  void set_result(Args &&...args) {
    constexpr auto is_constructable = 
      std::is_constructible_v<T, Args...> || std::is_same_v<void, T>;
    static_assert(
      is_constructable, "<<T>> is not constructable from <<arguments...>>");

    throw_if_empty("set_result - error.");

    producer_state_->set_result(std::forward<Args>(args)...);
    producer_state_.reset();  // publishes the result
  }

  void set_exception(std::exception_ptr exception_ptr) {
    throw_if_empty("set_exception - error.");

    if (!static_cast<bool>(exception_ptr)) {
      throw std::invalid_argument("set_exception - is null");
    }

    producer_state_->set_exception(exception_ptr);
    producer_state_.reset();  // publishes the result
  }

  template <typename CT, class ...Args>
  void set_from_function(CT &&callable, Args &&...args) noexcept {
    constexpr auto is_invokable = std::is_invocable_r_v<T, CT, Args...>;

    static_assert(
      is_invokable,
      "is not invokable or its return type can't be used to construct <<T>>");

    throw_if_empty("set_from_function error");
    producer_state_->from_callable(
      bind(std::forward<CT>(callable),
      std::forward<Args>(args)...));
    producer_state_.reset();  // publishes the result
  }

  Result<T> get_result() {
    throw_if_empty("get_result - failed.");

    if (!static_cast<bool>(consumer_state_)) {
      throw std::runtime_error("get_result - consumer empty.");
    }

    return Result<T>(std::move(consumer_state_));
  }
};

} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_BASIC_H_
