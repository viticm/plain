/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id consumer_context.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:12
 * @uses The concurrency result comsumer context detail.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_CONSUMER_CONTEXT_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_CONSUMER_CONTEXT_H_

#include "plain/concurrency/result/detail/config.h"
#include <semaphore>

namespace plain::concurrency {
namespace result::detail {

class PLAIN_API AwaitViaFunctor {

 public:
  AwaitViaFunctor(
    coroutine_handle<void> caller_handle, bool *interrupted) noexcept;
  AwaitViaFunctor(AwaitViaFunctor &&rhs) noexcept;
  ~AwaitViaFunctor() noexcept;

  void operator()() noexcept;

 private:
  coroutine_handle<void> caller_handle_;
  bool *interrupted_;

};

class PLAIN_API WhenAnyContext {

 public:
  WhenAnyContext(coroutine_handle<void> caller_handle_) noexcept;

 public:
  bool any_result_finished() const noexcept;
  bool finish_processing() noexcept;
  const StateBasic *completed_result() const noexcept;
  void try_resume(StateBasic &completed_result) noexcept;
  bool resume_inline(StateBasic &completed_result) noexcept;

 private:
  std::atomic<const StateBasic *> status_;
  coroutine_handle<void> coroutine_handle_;
  static const StateBasic *kProcessing;
  static const StateBasic *kDoneProcessing;

};

class PLAIN_API ConsumerContext {

 public:
  ~ConsumerContext() noexcept;

 public:
  void clear() noexcept;
  void resume_consumer(StateBasic &self) const;
  void set_await_handle(coroutine_handle<void> caller_handle) noexcept;
  void set_wait_for_context(
    const std::shared_ptr<std::binary_semaphore> &wait_ctx) noexcept;
  void set_when_any_context(
    const std::shared_ptr<WhenAnyContext> &when_any_ctx) noexcept;
  void set_shared_context(
    const std::shared_ptr<SharedStateBasic> &shared_ctx) noexcept;

 private:
  void destroy() noexcept;

 private:
  union storage {
    coroutine_handle<void> caller_handle;
    std::shared_ptr<WhenAnyContext> when_any_ctx;
    std::weak_ptr<SharedStateBasic> shared_ctx;

    storage() noexcept {}
    ~storage() noexcept {}
  };

 private:
  ConsumerStatus status_{ConsumerStatus::Idle};
  storage storage_;

};

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_CONSUMER_CONTEXT_H_
