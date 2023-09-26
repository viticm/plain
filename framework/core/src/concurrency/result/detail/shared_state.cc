#include "plain/concurrency/result/detail/shared_state.h"

using plain::concurrency::result::detail::SharedStateBasic;
using plain::concurrency::result::detail::SharedAwaitContext;
using plain::concurrency::ResultStatus;

SharedAwaitContext *SharedStateBasic::ready_constant() noexcept {
  return reinterpret_cast<SharedAwaitContext *>(-1);
}

ResultStatus SharedStateBasic::status() const noexcept {
  return status_.load(std::memory_order_acquire);
}

bool SharedStateBasic::await(SharedAwaitContext &awaiter) noexcept {
  while (true) {
    auto awaiter_before = awaiters_.load(std::memory_order_acquire);
    if (awaiter_before == ready_constant()) {
      return false;
    }
    awaiter.next = awaiter_before;
    const auto swapped = awaiters_.compare_exchange_weak(
      awaiter_before, &awaiter, std::memory_order_acq_rel);
    if (swapped) return true;
  }

  return false;
}

void SharedStateBasic::wait() noexcept {
  sys::detail::atomic_wait(
    status_, ResultStatus::Idle, std::memory_order_acquire);
}
