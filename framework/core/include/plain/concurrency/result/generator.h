/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id generator.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/10 15:09
 * @uses The plain concurrency result generator.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_GENERATOR_H_
#define PLAIN_CONCURRENCY_RESULT_GENERATOR_H_

#include "plain/concurrency/result/config.h"
#include <utility>
#include "plain/concurrency/result/detail/generator_state.h"

namespace plain::concurrency {
namespace result {

template <typename T>
requires is_result_generator_type<T>
class Generator {

 public:
  using promise_type = detail::GeneratorState<T>;
  using iterator = detail::GeneratorIterator<T>;

 public:
  Generator(coroutine_handle<promise_type> handle) noexcept
    : coroutine_handle_{handle} {
  }

  Generator(Generator &&rhs) noexcept
    : coroutine_handle_{std::exchange(rhs.coroutine_handle_, {})} {}

  ~Generator() noexcept {
    if (static_cast<bool>(coroutine_handle_)) {
      coroutine_handle_.destroy();
    }
  }

  Generator(const Generator &) = delete;
  Generator &operator=(const Generator &) = delete;
  Generator &operator=(Generator &&) = delete;

 public:
  explicit operator bool() const noexcept {
    return static_cast<bool>(coroutine_handle_);
  }

 public:
  iterator begin() noexcept {
    assert(static_cast<bool>(coroutine_handle_));
    assert(!coroutine_handle_.done());
    coroutine_handle_.resume();
    if (coroutine_handle_.done()) {
      coroutine_handle_.promise().log_if_exception();
    }
    return iterator{coroutine_handle_};
  }

  iterator end() noexcept {
    return {};
  }

 private:
  coroutine_handle<promise_type> coroutine_handle_;

};

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_GENERATOR_H_
