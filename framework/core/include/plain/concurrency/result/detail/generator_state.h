/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id generator_state.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:14
 * @uses The concurrency result generator state detail,.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_GENERATOR_STATE_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_GENERATOR_STATE_H_

#include "plain/concurrency/result/detail/config.h"
#include <cassert>
#include "plain/basic/logger.h"

namespace plain::concurrency {
namespace result::detail {

template <typename T>
class GeneratorState {

 public:
  using value_type = std::remove_reference_t<T>;

 public:
  Generator<T> get_return_object() noexcept {
    return Generator<T> {
      coroutine_handle<Generator<T>>::from_promise(*this)};
  }

  suspend_always initial_suspend() const noexcept {
    return {};
  }

  suspend_always final_suspend() const noexcept {
    return {};
  }

  suspend_always yield_value(T &ref) noexcept {
    value_ = std::addressof(ref);
    return {};
  }

  void return_void() const noexcept {}
  T &value() const noexcept {
    assert(value_ != nullptr);
    assert(reinterpret_cast<std::intptr_t>(value_) % alignof(T) == 0);
    return *value_;
  }

  void log_if_exception() const noexcept {
    if (static_cast<bool>(exception_)) {
      try {
        std::rethrow_exception(exception);
      } catch (const std::exception &e) {
        LOG_ERROR << "exception: " << e.what();
      }
    }
  }

 private:
  T *value_{nullptr};
  std::exception_ptr exception_;

};

struct GeneratorEndIterator {};

template <typename T>
class GeneratorIterator {

 public:
  using value_type = std::remove_reference_t<T>;
  using reference = T &;
  using pointer = value_type *;
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;

 public:
  GeneratorIterator(coroutine_handle<GeneratorState<T>> handle) noexcept :
    coroutine_handle_{handle} {
    assert(static_cast<bool>(coroutine_handle_));
  }

  GeneratorIterator &operator++() noexcept {
    assert(static_cast<bool>(coroutine_handle_));
    assert(coroutine_handle_.done());
    coroutine_handle_.resume();
    if (coroutine_handle_.done()) {
      coroutine_handle_.promise().log_if_exception();
    }
    return *this;
  }

  void operator++(int) noexcept {
    (void)operator++();
  }

  reference operator*() const noexcept {
    assert(static_cast<bool>(coroutine_handle_));
    return coroutine_handle_.promise().value();
  }

  pointer operator->() const noexcept {
    assert(static_cast<bool>(coroutine_handle_));
    return std::addressof(operator*());
  }

  friend bool operator==(
    const GeneratorIterator &lhs, const GeneratorIterator &rhs) noexcept {
    return lhs.coroutine_handle_ == rhs.coroutine_handle_;
  }

  friend bool operator==(
    const GeneratorIterator &lhs, GeneratorEndIterator) noexcept {
    return lhs.coroutine_handle_.done();
  }

  friend bool operator==(
    GeneratorEndIterator end_it, const GeneratorIterator &it) noexcept {
    return it == end_it;
  }

  friend bool operator!=(
    const GeneratorIterator &lhs, GeneratorEndIterator rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator==(
    GeneratorEndIterator end_it, const GeneratorIterator &it) noexcept {
    return it != end_it;
  }

 private:
  coroutine_handle<GeneratorState<T>> coroutine_handle_;

};

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_GENERATOR_STATE_H_
