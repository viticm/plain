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
#include "plain/concurrency/result/detail/state.h"

namespace plain::concurrency {
namespace result::detail {

struct shared_helper {

template <typename T>
static consumer_state_ptr_t<T> get_state(Result<T>& result) noexcept {
  return std::move(result.state_);
}

};

struct PLAIN_API SharedWaitContext {
  SharedWaitContext *next{nullptr};
  coroutine_handle<void > callable;
};

class PLAIN_API SharedStateBasic {

 public:
  virtual ~SharedStateBasic() noexcept = default;

 public:
  static SharedWaitContext *ready_constant() noexcept;

 public:
  virtual void on_finished() noexcept = 0;

 public:
  ResultStatus status() const noexcept;
  void wait() noexcept;
  bool await(SharedWaitContext &awaiter) noexcept;
  template <typename Rep, typename Period = std::ratio<1>>
  ResultStatus wait_for(std::chrono::duration<Rep, Period> duration) {
    const auto time_point = std::chrono::system_clock::now() + duration;
    return wait_until(time_point);
  }
  template <typename Clock, typename Duration = typename Clock::duration>
  ResultStatus wait_until(
      const std::chrono::time_point<Clock, Duration>& timeout_time) {
    while ((status() == ResultStatus::Idle) && (Clock::now() < timeout_time)) {
      const auto res = semaphore_.try_acquire_until(timeout_time);
      UNUSED(res);
    }
    return status();
  }

 protected:
  std::atomic<ResultStatus> status_{ResultStatus::Idle};
  std::atomic<SharedWaitContext *> awaiters_{nullptr};
  std::counting_semaphore<> semaphore_{0};

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
    state_->reset();
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
    status_.store(state_->status(), std::memory_order_release);
    status_.notify_all();

    semaphore_.release(semaphore_.max() / 2);
    auto kResultReady = ready_constant();
    auto awaiters = awaiters_.exchange(kResultReady, std::memory_order_acq_rel);
    
    SharedWaitContext *current = awaiters;
    SharedWaitContext *prev = nullptr, *next = nullptr;
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
