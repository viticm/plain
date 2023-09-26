#include "plain/concurrency/result/detail/consumer_context.h"
#include <utility>
#include "plain/concurrency/result/detail/shared_state.h"

namespace plain::concurrency {
namespace result::detail {

namespace {

template <typename T, typename ...Args>
void build(T &o, Args &&...args) noexcept {
  new (std::addressof(o)) T(std::forward<Args>(args)...);
}

template <typename T>
void destroy(T &o) noexcept {
  o.~T();
}

} // unname namespace

}
}

using plain::concurrency::result::detail::WhenAnyContext;
using plain::concurrency::result::detail::ConsumerContext;
using plain::concurrency::result::detail::AwaitViaFunctor;
using plain::concurrency::result::detail::StateBasic;

AwaitViaFunctor::AwaitViaFunctor(
  coroutine_handle<void> handle, bool *interrupted) noexcept :
  caller_handle_{handle}, interrupted_{interrupted} {
  assert(static_cast<bool>(caller_handle_));
  assert(!caller_handle_.done());
  assert(interrupted_ != nullptr);
}

AwaitViaFunctor::AwaitViaFunctor(AwaitViaFunctor &&rhs) noexcept :
  caller_handle_(std::exchange(rhs.caller_handle_, {})),
  interrupted_(std::exchange(rhs.interrupted_, nullptr)) {

}

AwaitViaFunctor::~AwaitViaFunctor() noexcept {
  if (interrupted_ == nullptr) return;
  *interrupted_ = true;
  caller_handle_();
}

void AwaitViaFunctor::operator()() noexcept {
  assert(interrupted_ != nullptr);
  interrupted_ = nullptr;
  caller_handle_();
}

const StateBasic *WhenAnyContext::kProcessing =
  reinterpret_cast<StateBasic *>(-1);
const StateBasic *WhenAnyContext::kDoneProcessing = nullptr;

WhenAnyContext::WhenAnyContext(coroutine_handle<void> handle) noexcept :
  status_(kProcessing), coroutine_handle_(handle) {
  assert(static_cast<bool>(coroutine_handle_));
  assert(!coroutine_handle_.done());
}

bool WhenAnyContext::any_result_finished() const noexcept {
  const auto status = status_.load(std::memory_order_acquire);
  assert(status != kDoneProcessing);
  return status != kProcessing;
}

bool WhenAnyContext::finish_processing() noexcept {
  assert(status_.load(std::memory_order_relaxed) != kDoneProcessing);
  auto expected_state = kProcessing;
  const auto r = status_.compare_exchange_strong(
    expected_state, kDoneProcessing, std::memory_order_acq_rel);

  return r;
}

void WhenAnyContext::try_resume(StateBasic &completed_result) noexcept {
  while (true) {
    auto status = status_.load(std::memory_order_acquire);
    if (status != kProcessing && status != kDoneProcessing) {
      return;
    }
    if (status == kDoneProcessing) {
      const auto swapped = status_.compare_exchange_strong(
        status, &completed_result, std::memory_order_acq_rel);
      if (!swapped) return;
      coroutine_handle_();
      return;
    }
    assert(status == kProcessing);
    const auto r = status_.compare_exchange_strong(
      status, &completed_result, std::memory_order_acq_rel);
    if (r) return;

  }
}

bool WhenAnyContext::resume_inline(StateBasic &completed_result) noexcept {
  auto status = status_.load(std::memory_order_acquire);
  assert(status != kDoneProcessing);

  if (status != kProcessing) return false;

  status_.compare_exchange_strong(
    status, &completed_result, std::memory_order_acq_rel);
  return false;
}

const StateBasic *WhenAnyContext::completed_result() const noexcept {
  return status_.load(std::memory_order_acquire);
}

ConsumerContext::~ConsumerContext() noexcept {
  destroy();
}

void ConsumerContext::destroy() noexcept {
  switch (status_) {
    case ConsumerStatus::Idle:
      break;
    case ConsumerStatus::Await:
      detail::destroy(storage_.caller_handle);
      break;
    case ConsumerStatus::WhenAny:
      detail::destroy(storage_.when_any_ctx);
      break;
    case ConsumerStatus::Shared:
      detail::destroy(storage_.shared_ctx);
      break;
    default:
      assert(false);
      break;
  }
}

void ConsumerContext::clear() noexcept {
  destroy();
  status_ = ConsumerStatus::Idle;
}

void ConsumerContext::set_await_handle(
  coroutine_handle<void> caller_handle) noexcept {
  assert(status_ == ConsumerStatus::Idle);

  status_ = ConsumerStatus::Await;
  detail::build(storage_.caller_handle, caller_handle);
}

void ConsumerContext::set_when_any_context(
  const std::shared_ptr<WhenAnyContext> &when_any_ctx) noexcept {
  assert(status_ == ConsumerStatus::Idle);

  status_ = ConsumerStatus::WhenAny;
  detail::build(storage_.when_any_ctx, when_any_ctx);
}

void ConsumerContext::set_shared_context(
  const std::shared_ptr<SharedStateBasic> &shared_ctx) noexcept {
  assert(status_ == ConsumerStatus::Idle);

  status_ = ConsumerStatus::Shared;
  detail::build(storage_.shared_ctx, shared_ctx);
}

void ConsumerContext::resume_consumer(StateBasic &self) const {
  switch (status_) {
    case ConsumerStatus::Idle: {
      return;
    }
    case ConsumerStatus::Await: {
      auto caller_handle = storage_.caller_handle;
      assert(static_cast<bool>(caller_handle));
      assert(!caller_handle.done());
      return caller_handle();
    }
    case ConsumerStatus::WhenAny: {
      const auto when_any_ctx = storage_.when_any_ctx;
      return when_any_ctx->try_resume(self);
    }
    case ConsumerStatus::Shared: {
      const auto weak_shared_ctx = storage_.shared_ctx;
      const auto shared_ctx = weak_shared_ctx.lock();
      if (static_cast<bool>(shared_ctx)) {
        shared_ctx->on_finished();
      }
      return;
    }
  }

  assert(false);
}
