#include "plain/concurrency/result/detail/state.h"
#include "plain/sys/atomic_wait.h"
#include "plain/concurrency/result/detail/shared_state.h"

using plain::concurrency::result::detail::StateBasic;
using plain::concurrency::result::detail::ProcessStatus;

void StateBasic::assert_done() const noexcept {
  assert(process_status_.load(std::memory_order_acquire)
    == ProcessStatus::ProducerDone);
}

void StateBasic::wait() noexcept {
  const auto status = process_status_.load(std::memory_order_acquire);
  if (status == ProcessStatus::ProducerDone) return;

  auto expected_status = ProcessStatus::Idle;
  const auto idle = process_status_.compare_exchange_strong(
    expected_status,
    ProcessStatus::ConsumerWaiting,
    std::memory_order_acq_rel,
    std::memory_order_acquire);
  if (!idle) assert_done();
  while (true) {
    if (process_status_.load(std::memory_order_acquire) 
        == ProcessStatus::ProducerDone)
      break;
    process_status_.wait(
      ProcessStatus::ConsumerWaiting, std::memory_order_acquire);
  }
}
  
ProcessStatus StateBasic::wait_for(std::chrono::milliseconds ms) noexcept {
    const auto state1 = process_status_.load(std::memory_order_acquire);
    if (state1 == ProcessStatus::ProducerDone)
      return ProcessStatus::ProducerDone;


    auto expected_status1 = ProcessStatus::Idle; // Reference.
    const auto idle1 = process_status_.compare_exchange_strong(
      expected_status1, ProcessStatus::ConsumerSet,
      std::memory_order_acq_rel, std::memory_order_acquire);
    if (!idle1) {
      assert_done();
      return ProcessStatus::ProducerDone;
    }

    ms += std::chrono::milliseconds(2);
    const auto res = plain::sys::detail::atomic_wait_for(
      process_status_, ProcessStatus::ConsumerWaiting, ms,
      std::memory_order_acquire);
    if (res == plain::sys::detail::AtomicWaitStatus::Success) {
      assert_done();
      return ProcessStatus::ProducerDone;
    }
    assert(res == plain::sys::detail::AtomicWaitStatus::Timeout);
    
    auto expected_status2 = ProcessStatus::ConsumerWaiting;
    const auto idle2 = process_status_.compare_exchange_strong(
      expected_status2, ProcessStatus::Idle,
      std::memory_order_acq_rel, std::memory_order_acquire);
    if (!idle2) {
      return ProcessStatus::Idle;
    }
    assert_done();
    return ProcessStatus::ProducerDone;
}

bool StateBasic::await(coroutine_handle<void> caller_handle) noexcept {
  const auto status = process_status_.load(std::memory_order_acquire);
  if (status == ProcessStatus::ProducerDone) return false;

  consumer_.set_await_handle(caller_handle);
  
  auto expected_status = ProcessStatus::Idle;
   const auto idle = process_status_.compare_exchange_strong(
    expected_status,
    ProcessStatus::ConsumerSet,
    std::memory_order_acq_rel,
    std::memory_order_acquire);
  if (!idle) assert_done();
  return idle;
}

ProcessStatus StateBasic::when_any(
  const std::shared_ptr<WhenAnyContext> &when_any_state) noexcept {
  const auto status = process_status_.load(std::memory_order_acquire);
  if (status == ProcessStatus::ProducerDone) return status;
  consumer_.set_when_any_context(when_any_state);
  auto expected_status = ProcessStatus::Idle;
  const auto idle = process_status_.compare_exchange_strong(
    expected_status,
    ProcessStatus::ConsumerSet,
    std::memory_order_acq_rel,
    std::memory_order_acquire);
  if (!idle) assert_done();
  return status;
}

void StateBasic::share(
  const std::shared_ptr<SharedStateBasic> &shared_result_state) noexcept {
  const auto status = process_status_.load(std::memory_order_acquire);
  if (status == ProcessStatus::ProducerDone)
    return shared_result_state->on_finished();
  auto expected_status = ProcessStatus::Idle;
  const auto idle = process_status_.compare_exchange_strong(
    expected_status,
    ProcessStatus::ConsumerSet,
    std::memory_order_acq_rel,
    std::memory_order_acquire);
  if (!idle) return;
  assert_done();
  shared_result_state->on_finished();
}

void StateBasic::try_rewind_consumer() noexcept {
  const auto status = process_status_.load(std::memory_order_acquire);
  if (status != ProcessStatus::ConsumerSet) return;
  auto expected_status = ProcessStatus::ConsumerSet;
  const auto consumer = process_status_.compare_exchange_strong(
    expected_status,
    ProcessStatus::ConsumerSet,
    std::memory_order_acq_rel,
    std::memory_order_acquire);
  if (!consumer) {
    assert_done();
    return;
  }
  consumer_.clear();
}
