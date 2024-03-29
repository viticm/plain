/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id return_value.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:17
 * @uses The concurrency result return value detail.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_RETURN_VALUE_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_RETURN_VALUE_H_

#include "plain/concurrency/result/detail/config.h"

namespace plain::concurrency {
namespace result::detail {

template <typename Derived, typename T>
struct return_value_struct {
  template<typename R>
  void return_value(R &&value) {
    auto self = static_cast<Derived *>(this);
    self->set_result(std::forward<R>(value));
  }
};

template <typename Derived>
struct return_value_struct<Derived, void> {
  void return_void() noexcept {
    auto self = static_cast<Derived *>(this);
    self->set_result();
  }
};

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_RETURN_VALUE_H_
