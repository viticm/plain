/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id inline.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/04 17:32
 * @uses The concurrency inline executor implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_INLINE_H_
#define PLAIN_CONCURRENCY_EXECUTOR_INLINE_H_

#include "plain/concurrency/executor/config.h"
#include "plain/concurrency/executor/basic.h"

namespace plain::concurrency {
namespace executor {

class PLAIN_API Inline : public Basic {

 public:
  Inline() noexcept : Basic("inline"), abort_{false} {}

 public:
  void enqueue(Task task) override {
    throw_if_aborted();
    task();
  }

  void enqueue(std::span<Task> tasks) override {
    throw_if_aborted();
    for (auto &task : tasks) {
      task();
    }
  }

  int32_t max_concurrency_level() const noexcept override {
    return 1;
  }

  void shutdown() override {
    abort_.store(true, std::memory_order_relaxed);
  }

  bool shutdown_requested() const override {
    return abort_.load(std::memory_order_relaxed);
  }

 private:
  std::atomic_bool abort_;

 private:
  void throw_if_aborted() const {
    if (abort_.load(std::memory_order_relaxed)) {
      detail::throw_runtime_shutdown_exception(name_);
    }
  }

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_INLINE_H_
