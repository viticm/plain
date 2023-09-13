/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/31 17:48
 * @uses The concurrency executor basic implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_BASIC_H_
#define PLAIN_CONCURRENCY_EXECUTOR_BASIC_H_

#include "plain/concurrency/executor/config.h"
#include <span>
#include "plain/basic/bind.h"
#include "plain/concurrency/result/basic.h"
#include "plain/concurrency/task.h"

namespace plain::concurrency {
namespace executor {

namespace detail {

[[noreturn]] PLAIN_API void 
throw_runtime_shutdown_exception(std::string_view executor_name);
PLAIN_API std::string make_executor_worker_name(std::string_view name);

} // namespace detail

class PLAIN_API Basic {

 public:
  Basic(std::string_view name) : name_{name} {}
  virtual ~Basic() = default;

 public:
  virtual void enqueue(Task task) = 0;
  virtual void enqueue(std::span<Task> tasks) = 0;
  virtual int32_t max_concurrency_level() const noexcept = 0;
  virtual bool shutdown_requested() const = 0;
  virtual void shutdown() = 0;

 public:
  template <typename T, typename ...Args>
  void post(T &&callable, Args &&...args) {
    return do_post<Basic>(
      std::forward<T>(callable), std::forward<Args>(args)...);
  }

  template <typename T, typename ...Args>
  void submit(T &&callable, Args &&...args) {
    return do_submit<Basic>(
      std::forward<T>(callable), std::forward<Args>(args)...);
  }

  template <typename T>
  void bluk_post(std::span<T> callables) {
    return do_bulk_post<Basic>(callables);
  }

  template <typename T, typename RT = std::invoke_result_t<T>>
  std::vector<Result<RT>> bulk_submit(std::span<T> callables) {
    return do_bulk_submit<Basic>(callables);
  }

 public:
   const std::string name_;

 protected:
  template <typename ET, typename CT, typename ...Args>
  void do_post(CT &&callable, Args &&...args) {
    static_assert(std::is_invocable_v<CT, Args ...>, "Not invoke with args");
    static_cast<ET *>(this)->enqueue(
      bind_with_try_catch(
        std::forward<CT>(callable), std::forward<Args>(args)...));
  }

  template <typename ET, typename CT, typename ...Args>
  void do_submit(CT &&callable, Args &&...args) {
    static_assert(std::is_invocable_v<CT, Args ...>, "Not invoke with args");
    using return_type = typename std::invoke_result_t<CT, Args ...>;
    return submit_bridge<return_type>(
      *static_cast<ET *>(this), std::forward<CT>(callable),
      std::forward<Args>(args)...);
  }

  template <typename ET, typename CT>
  void do_bulk_post(std::span<CT> callables) {
    assert(!callables.empty());

    std::vector<Task> tasks;
    tasks.reserve(callables.size());

    for (auto &callable : callables) {
      tasks.emplace_back(bind_with_try_catch(std::move(callable)));
    }
    std::span<Task> span = tasks;
    static_cast<ET *>(this)->enqueue(span);
  }

  template <typename ET, typename CT, typename RT = std::invoke_result_t<CT>>
  std::vector<Result<RT>> do_bulk_submit(std::span<CT> callables) {
    std::vector<Task> accumulator;
    accumulator.reserve(callables.size());

    std::vector<Result<RT>> results;
    results.reserve(callables.size());

    for (auto &callable : callables) {
      results.emplace_back(
        bulk_submit_bridge<CT>(accumulator, std::move(callable)));
    }
    
    assert(!accumulator.empty());
    std::span<Task> span = accumulator;
    static_cast<ET *>(this)->enqueue(span);
    return results;
  }

 private:
  struct accumulating_awaitable {
    std::vector<Task> &accumulator;
    bool interrupted_{false};

    accumulating_awaitable(std::vector<Task> &accumulator) noexcept :
      accumulator(accumulator) {}
    constexpr bool await_ready() const noexcept {
      return false;
    }
    void await_suspend(coroutine_handle<void> handle) noexcept {
      try {
        accumulator.emplace_back(
          result::detail::AwaitViaFunctor(handle, &interrupted_));
      } catch(...) {
        // do nothing
      }
    }
  };

 private:

  template <typename RT, typename ET, typename CT, typename ...Args>
  static Result<RT>
  submit_bridge(executor_tag, ET &, CT callable, Args... args) {
    co_return callable(args...);
  }

  template <typename CT, typename RT = typename std::invoke_result_t<CT>>
  static Result<RT>
  bulk_submit_bridge(std::vector<Task> &accumulator, CT callable) {
    co_await accumulating_awaitable(accumulator);
    co_return callable();
  }

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_BASIC_H_
