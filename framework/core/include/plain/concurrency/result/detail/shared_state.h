/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id shared_state.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:18
 * @uses The concurrency result shared state detail.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_SHARED_STATE_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_SHARED_STATE_H_

#include "plain/concurrency/result/detail/config.h"
#include "plain/sys/atomic_wait.h"
#include "plain/concurrency/result/detail/state.h"

namespace plain::concurrency {
namespace result::detail {

struct shared_helper {

template <typename T>
static consumer_state_ptr_t<T> get_state(Result<T>& result) noexcept {
  return std::move(result.state_);
}

};

struct PLAIN_API SharedAwaitContext {
  SharedAwaitContext *next{nullptr};
  coroutine_handle<void > callable;
};

class PLAIN_API SharedStateBasic {

 public:
  virtual ~SharedStateBasic() noexcept = default;

 public:
  static SharedAwaitContext *ready_constant() noexcept;

 public:
  virtual void on_finished() noexcept = 0;

 public:
  ResultStatus status() const noexcept;
  void wait() noexcept;
  bool await(SharedAwaitContext &awaiter) noexcept;
  template <typename Rep, typename Period = std::ratio<1>>
  ResultStatus wait_for(std::chrono::duration<Rep, Period> duration) {
    using plain::sys::detail::atomic_wait_for;
    const auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration) +
      std::chrono::milliseconds(2);
    atomic_wait_for(status_, ResultStatus::Idle, ms, std::memory_order_acquire);
    return status();
  }
  template <typename Clock, typename Duration = typename Clock::duration>
  ResultStatus wait_until(
      const std::chrono::time_point<Clock, Duration>& timeout_time) {
    const auto time_now = Clock::now();
    if (time_now >= timeout_time) return status();
    return wait_for(timeout_time - time_now);
  }

 protected:
  std::atomic<ResultStatus> status_{ResultStatus::Idle};
  std::atomic<SharedAwaitContext *> awaiters_{nullptr};

};

template <typename T>
class SharedState : public SharedStateBasic {

 public:
  SharedState(consumer_state_ptr_t<T> state)
    noexcept : state_(std::move(state)) {
    assert(static_cast<bool>(state_));
  }

  ~SharedState() {
    assert(static_cast<bool>(state_));
    state_->try_rewind_consumer();
    state_.reset();
  }

 public:
  void share(const std::shared_ptr<SharedStateBasic> &self) noexcept {
    assert(static_cast<bool>(state_));
    state_->share(self);
  }

  std::add_lvalue_reference_t<T> get() {
    assert(static_cast<bool>(state_));
    return state_->get_ref();
  }

  void on_finished() noexcept override {
    using plain::sys::detail::atomic_notify_all;
    status_.store(state_->status(), std::memory_order_release);
    atomic_notify_all(status_);

    auto kResultReady = ready_constant();
    auto awaiters = awaiters_.exchange(kResultReady, std::memory_order_acq_rel);
    
    SharedAwaitContext *current = awaiters;
    SharedAwaitContext *prev = nullptr, *next = nullptr;
    while (current != nullptr) {
      next = current->next;
      current->next = prev;
      prev = current;
      current = next;
    }
    
    awaiters = prev;

    while (awaiters != nullptr) {
      assert(static_cast<bool>(awaiters->callable));
      auto callable = awaiters->callable;
      awaiters = awaiters->next;
      callable();
    }
  }

 private:
  consumer_state_ptr_t<T> state_;

};

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_SHARED_STATE_H_
