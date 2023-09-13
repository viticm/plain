/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id derivable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/04 14:17
 * @uses The concurrency executor derivable implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_DERIVABLE_H_
#define PLAIN_CONCURRENCY_EXECUTOR_DERIVABLE_H_

#include "plain/concurrency/executor/config.h"
#include "plain/concurrency/executor/basic.h"

namespace plain::concurrency {
namespace executor {

template <typename T>
class Derivable : public Basic {

 public:
  Derivable(std::string_view name) : Basic{name} {}

 public:
  template <typename CT, typename ...Args>
  void post(CT &&callable, Args &&...args) {
    do_post<T>(std::forward<CT>(callable), std::forward<Args>(args)...);
  }

  template <typename CT, typename ...Args>
  void submit(CT &&callable, Args &&...args) {
    do_submit<T>(std::forward<CT>(callable), std::forward<Args>(args)...);
  }

  template <typename CT>
  void bulk_post(std::span<CT> callables) {
    do_bulk_post<T>(callables);
  }

  template <class CT, class RT = std::invoke_result_t<CT>>
  std::vector<Result<RT>> bulk_submit(std::span<CT> callables) {
    return do_bulk_submit<T>(callables);
  }

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_DERIVABLE_H_
