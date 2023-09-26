/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id make.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/07 16:41
 * @uses The concurrency make result functions.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_MAKE_H_
#define PLAIN_CONCURRENCY_RESULT_MAKE_H_

#include "plain/concurrency/result/config.h"
#include <type_traits>
#include "plain/basic/logger.h"
#include "plain/concurrency/result/basic.h"

namespace plain::concurrency {
namespace result {

template <typename T, typename ...Args>
Result<T> make_ready(Args &&...args) {
  static_assert(std::is_constructible_v<T, Args...> || std::is_same_v<T, void>,
                "make_ready constructible check failed");
  static_assert(std::is_same_v<T, void> ? (sizeof...(Args) == 0) : true,
                "make_ready overload does not accept any argument");
  detail::producer_state_ptr_t<T> promise(new detail::State<T>());
  detail::consumer_state_ptr_t<T> state{promise.get()};
  promise->set_result(std::forward<Args>(args)...);
  promise.reset();
  return {std::move(state)};
};

template <typename T>
Result<T> make_exceptional(std::exception_ptr exception) {
  if (!static_cast<bool>(exception))
    throw std::invalid_argument("make_exceptional exception null");
  detail::producer_state_ptr_t<T> promise(new detail::State<T>());
  detail::consumer_state_ptr_t<T> state{promise.get()};
  promise->set_exception(exception);
  promise.reset();
  return {std::move(state)};
}

template <typename T, typename E>
Result<T> make_exceptional(E exception) {
  return make_exceptional<T>(std::make_exception_ptr(exception));
}

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_MAKE_H_
