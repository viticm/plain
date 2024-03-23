/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id resume_on.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/07 15:48
 * @uses The concurrency result resume on.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_RESUME_ON_H_
#define PLAIN_CONCURRENCY_RESULT_RESUME_ON_H_

#include "plain/concurrency/result/config.h"
#include <type_traits>
#include <cassert>
#include "plain/basic/logger.h"
#include "plain/concurrency/result/detail/consumer_context.h"

namespace plain::concurrency {
namespace result {

namespace detail {

template <typename T>
class ResumeOnAwaitable : public suspend_always {

 public:
  ResumeOnAwaitable(T &executor) noexcept : executor_{executor} {
  }

  ResumeOnAwaitable(const ResumeOnAwaitable &) = delete;
  ResumeOnAwaitable(ResumeOnAwaitable &&) = delete;

  ResumeOnAwaitable &operator=(const ResumeOnAwaitable &) = delete;
  ResumeOnAwaitable &operator=(ResumeOnAwaitable &&) = delete;

 public:
  void await_suspend(coroutine_handle<void> handle) noexcept {
    try {
      executor_.post(AwaitViaFunctor{handle, &interrupted_});
    } catch(...) {
      // do nothing
    }
  }

  void await_resume() const {
    if (interrupted_)
      throw std::runtime_error(
        "await_resume - associated task was interrupted abnormally");
  }

 private:
  T &executor_;
  bool interrupted_{false};

};

} // namespace detail

template <typename T>
auto resume_on(std::shared_ptr<T> executor) {
  static_assert(std::is_base_of_v<executor::Basic, T>,
                "resume_on given executor does not derive from Executor");
  if (!executor) {
    throw std::invalid_argument("resume_on - given executor is null.");
  }
  return detail::ResumeOnAwaitable{*executor.get()};
}

template <typename T>
auto resume_on(T &executor) noexcept {
  return detail::ResumeOnAwaitable{executor};
}

} // namespace result
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_RESUME_ON_H_
