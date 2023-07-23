/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id state.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:19
 * @uses The concurrency result state detail.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_STATE_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_STATE_H_

#include "plain/concurrency/result/detail/config.h"
#include <cassert>
#include <semaphore>

namespace plain::concurrency {
namespace result::detail {

class PLAIN_API StateBasic {

 public:
  void wait() noexcept;
  bool await(coroutine_handle<void> caller_handle) noexcept;
  ProcessStatus when_any(
    const std::shared_ptr<WhenAnyContext> &when_any_state) noexcept;

  void share(const std::shared_ptr<SharedStateBasic> &shared_state) noexcept;

  void try_rewind_consumer() noexcept;

 protected:
  std::atomic<ProcessStatus> process_status_{ProcessStatus::Idle};
  ConsumerContext consumer_;
  coroutine_handle<void> done_handle_;

 protected:
  void assert_done() const noexcept;

};

template <typename T>
class State : StateBasic {

 public:
  template <typename ...Args>
  void set_result(Args &&...args) noexcept(
    noexcept(T(std::forward<Args>(args)...))) {
    producer_.build_result(std::forward<Args>(args)...);
  }

  void set_exception(const std::exception_ptr &exception) noexcept {
    assert(static_cast<bool>(exception));
    producer_.build_exception(exception);
  }

  ResultStatus status() const noexcept {
    const auto state = process_status_.load(std::memory_order_acquire);
    assert(state != ProcessStatus::ConsumerSet);
    if (state == ProcessStatus::Idle)
      return ResultStatus::Idle;
    return producer_.status();
  }

  template <typename Rep, typename Period = std::ratio<1>>
  ResultStatus wait_for(std::chrono::duration<Rep, Period> duration) {
    const auto state1 = process_status_.load(std::memory_order_acquire);
    if (state1 == ProcessStatus::ProducerDone)
      return producer_.status();

    const auto wait_ctx = std::make_shared<std::binary_semaphore>(0);
    consumer_.set_wait_for_context(wait_ctx);

    std::atomic_thread_fence(std::memory_order_release);

    auto expected_status1 = ProcessStatus::Idle; // Reference.
    const auto idle1 = process_status_.compare_exchange_strong(
      expected_status1, ProcessStatus::ConsumerSet,
      std::memory_order_acq_rel, std::memory_order_acquire);
    if (!idle1) {
      assert_done();
      return producer_.status();
    }
    
    if (wait_ctx->try_acquire_for(duration + std::chrono::milliseconds(1))) {
      const auto status = process_status_.load(std::memory_order_acquire);
      UNUSED(status);
      return producer_.status();
    }
    
    auto expected_status2 = ProcessStatus::ConsumerSet;
    const auto idle2 = process_status_.compare_exchange_strong(
      expected_status2, ProcessStatus::Idle,
      std::memory_order_acq_rel, std::memory_order_acquire);
    if (!idle2) {
      assert_done();
      return producer_.status();
    }
    consumer_.clear();
    return ResultStatus::Idle;
  }

  template <typename Clock, typename Duration = typename Clock::duration>
  ResultStatus wait_until(
    const std::chrono::time_point<Clock, Duration>& timeout_time) {
    const auto now = Clock::now();
    if (timeout_time <= now) {
      return status();
    }
    const auto diff = timeout_time - now;
    return wait_for(diff);
  }

  T get() noexcept {
    assert_done();
    return producer_.get();
  }

  std::add_lvalue_reference_t<T> get_ref() {
    assert_done();
    return producer_.get_ref();
  }
  
  template <typename F>
  void form_callable(F &&func) noexcept {
    using is_void = std::is_same<T, void>;
    try {
      from_callable(is_void {}, std::forward<F>(func));
    } catch(...) {
      set_exception(std::current_exception());
    }
  }

  void complete_producer(coroutine_handle<void> done_handle = {}) {
    const auto state_before = this->process_status_.exchange(
      ProcessStatus::ProducerDone, std::memory_order_acq_rel);
    assert(state_before != ProcessStatus::ProducerDone);
    switch (state_before) {
      case ProcessStatus::ConsumerSet: {
        return consumer_.resume_consumer(*this);
      }
      case ProcessStatus::Idle: {
        return;
      }
      case ProcessStatus::ConsumerWaiting: {
        return process_status_.notify_one();
      }
      case ProcessStatus::ConsumerDone: {
        return delete_self(this);
      }
      default:
        break;
    }
    assert(false);
  }

  void complete_consumer() noexcept {
    const auto state = this->process_status_.load(std::memory_order_acquire);
    if (state == ProcessStatus::ProducerDone) {
      return delete_self(this);
    }

    const auto state1 = this->process_status_.exchange(
      ProcessStatus::ConsumerDone, std::memory_order_acq_rel);
    assert(state1 != ProcessStatus::ConsumerSet);
    if (state1 == ProcessStatus::ProducerDone) {
      return delete_self(this);
    }
    assert(state1 == ProcessStatus::Idle);
  }

  void complete_joined_consumer() noexcept {
    assert_done();
    delete_self(this);
  }

 private:
  ProducerContext<T> producer_;

 private:
  static void delete_self(State<T> *state) noexcept {
    assert(state);
    auto done_handle = state->done_handle_;
    if (static_cast<bool>(done_handle)) {
      assert(done_handle.done());
      return done_handle.destroy();
    }
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
    delete state;
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
  }

  template <typename U>
  void from_callable(std::true_type, U &&func) {
    func();
    set_result();
  }

  template <typename U>
  void from_callable(std::false_type, U &&func) {
    set_result(func());
  }

};

// deleter object functions.
template <typename T>
struct consumer_state_deleter {
  void operator()(State<T> *state_ptr) const noexcept {
    assert(state_ptr != nullptr);
    state_ptr->complete_consumer();
  }
};

template <typename T>
struct joined_consumer_state_deleter {
  void operator()(State<T> *state_ptr) const noexcept {
    assert(state_ptr != nullptr);
    state_ptr->complete_joined_consumer();
  }
};

template <typename T>
struct producer_state_deleter {
  void operator()(State<T> *state_ptr) const {
    assert(state_ptr != nullptr);
    state_ptr->complete_producer();
  }
};

template <typename T>
using consumer_state_ptr_t = std::unique_ptr<
  State<T>, consumer_state_deleter<T>>;

template <typename T>
using joined_consumer_state_ptr_t = std::unique_ptr<
  State<T>, joined_consumer_state_deleter<T>>;

template <typename T>
using producer_state_ptr_t = std::unique_ptr<
  State<T>, producer_state_deleter<T>>;

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_STATE_H_
