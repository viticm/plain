/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id promise.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/10 17:18
 * @uses The concurrency result promise.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_PROMISE_H_
#define PLAIN_CONCURRENCY_RESULT_PROMISE_H_

#include "plain/concurrency/result/config.h"
#include "plain/concurrency/result/detail/lazy_state.h"
#include "plain/concurrency/result/detail/state.h"
#include "plain/concurrency/result/detail/return_value.h"

namespace plain::concurrency {
namespace result {

namespace detail {

template <typename T>
requires is_base_of_executor<T>
class initialy_rescheduled_promise {

 public:
  class initial_scheduling_awaiter : public suspend_always {
   
   public:
    template <typename PT>
    void await_suspend(coroutine_handle<PT> handle) {
      try {
        handle.promise().initial_executor_.post(
          AwaitViaFunctor{handle, &interrupted_});
      } catch(...) {
        // do nothing
      }
    }

    void await_resume() const {
      if (interrupted_)
        throw std::runtime_error("associated task was interrupted abnormally");
    }

   private:
    bool interrupted_{false};

  };

 public:
  template <typename ...Args>
  initialy_rescheduled_promise(executor_tag, T *executor_ptr, Args &&...)
    : initial_executor_{to_ref(executor_ptr)} {

  }

  template <typename ...Args>
  initialy_rescheduled_promise(executor_tag, T &executor, Args &&...) noexcept
    : initial_executor_{executor} {
  }

  template <typename ...Args>
  initialy_rescheduled_promise(
    executor_tag, std::shared_ptr<T> executor, Args &&...args)
    : initialy_rescheduled_promise(
      executor_tag{}, executor.get(), std::forward<Args>(args)...) {

  }
  
  template <typename CT, typename ...Args>
  initialy_rescheduled_promise(
    CT &&, executor_tag, std::shared_ptr<T> executor, Args &&...args) noexcept
    : initialy_rescheduled_promise(
      executor_tag{}, *executor, std::forward<Args>(args)...) {
  }

  initial_scheduling_awaiter initial_suspend() const noexcept {
    return {};
  }

 private:
  T &initial_executor_;

 private:
  static T &to_ref(T *ptr) {
    if (!ptr) throw std::invalid_argument("executor is null");
    return *ptr;
  }

};

struct initialy_resumed_promise {
  suspend_never initial_suspend() const noexcept {
    return {};
  }
};

struct null_promise {
  null get_return_object() const noexcept {
    return {};
  }
  
  suspend_never final_suspend() const noexcept {
    return {};
  }
  
  void unhandled_exception() const noexcept {

  }
  
  void return_void() const noexcept {

  }
};

struct publisher : public suspend_always {
  template <typename T>
  void await_suspend(coroutine_handle<T> handle) const noexcept {
    handle.promise().complete_producer(handle);
  }
};

template <typename T>
class coroutine_promise : public return_value_struct<coroutine_promise<T>, T> {

 public:
  template <typename ...Args>
  void set_result(Args &&...args) 
    noexcept(noexcept(T(std::forward<Args>(args)...))) {
    this->state_.set_result(std::forward<Args>(args)...);
  }

  void unhandled_exception() noexcept {
    this->state_.set_exception(std::current_exception());
  }

  Result<T> get_return_object() noexcept {
    return {&state_};
  }

  void complete_producer(coroutine_handle<void> done_handle) noexcept {
    this->state_.complete_producer(done_handle);
  }

  publisher final_suspend() const noexcept {
    return {};
  }

 private:
  State<T> state_;

};

template <typename T>
struct lazy_promise : LazyState<T>, 
  public return_value_struct<lazy_promise<T>, T> {};

struct initialy_resumed_null_result_promise : public initialy_resumed_promise,
  public null_promise {};

template <typename T>
struct initialy_resumed_result_promise : public initialy_resumed_promise,
  public coroutine_promise<T> {};

template <typename T>
struct initialy_rescheduled_null_result_promise :
  public initialy_rescheduled_promise<T>, public null_promise {
  using initialy_rescheduled_promise<T>::initialy_rescheduled_promise;
};

template <typename RT, typename ET>
struct initialy_rescheduled_result_promise :
  public initialy_rescheduled_promise<ET>, public coroutine_promise<RT> {
  using initialy_rescheduled_promise<ET>::initialy_rescheduled_promise;
};

} // namespace detail

} // namespace result
} // namespace plain::concurrency

// specail to std templates?
namespace std {

// No executor and null result
template <typename ...Args>
struct coroutine_traits<plain::concurrency::result::null, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_resumed_null_result_promise;
};

// No executor and result
template <typename T, typename ...Args>
struct coroutine_traits<plain::concurrency::Result<T>, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_resumed_result_promise<T>;
};

// Executor and null result
template <typename T, typename ...Args>
struct coroutine_traits<plain::concurrency::result::null,
  plain::concurrency::executor_tag, std::shared_ptr<T>, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_rescheduled_null_result_promise<T>;
};

template <typename T, typename ...Args>
struct coroutine_traits<plain::concurrency::result::null,
  plain::concurrency::executor_tag, T *, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_rescheduled_null_result_promise<T>;
};

template <typename T, typename ...Args>
struct coroutine_traits<plain::concurrency::result::null,
  plain::concurrency::executor_tag, T &, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_rescheduled_null_result_promise<T>;
};

// Executor and result
template <typename RT, typename ET, typename ...Args>
struct coroutine_traits<plain::concurrency::Result<RT>,
  plain::concurrency::executor_tag, std::shared_ptr<ET>, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_rescheduled_result_promise<RT, ET>;
};

template <typename RT, typename ET, typename ...Args>
struct coroutine_traits<plain::concurrency::Result<RT>,
  plain::concurrency::executor_tag, ET *, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_rescheduled_result_promise<RT, ET>;
};

template <typename RT, typename ET, typename ...Args>
struct coroutine_traits<plain::concurrency::Result<RT>,
  plain::concurrency::executor_tag, ET &, Args...> {
  using promise_type = 
    plain::concurrency::result::detail::initialy_rescheduled_result_promise<RT, ET>;
};

// Lazy
template <typename T, typename ...Args>
struct coroutine_traits<plain::concurrency::LazyResult<T>, Args...> {
  using promise_type =
    plain::concurrency::result::detail::lazy_promise<T>;
};

} // namespace std

#endif // PLAIN_CONCURRENCY_RESULT_PROMISE_H_
