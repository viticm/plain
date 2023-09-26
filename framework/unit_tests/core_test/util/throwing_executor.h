#ifndef PLAIN_CORE_TEST_RESULT_TEST_EXECUTORS_H_
#define PLAIN_CORE_TEST_RESULT_TEST_EXECUTORS_H_

#include "plain/concurrency/executor/basic.h"

namespace plain::tests {
  struct executor_enqueue_exception {};

  struct throwing_executor : public plain::concurrency::executor::Basic {
    throwing_executor() : concurrency::executor::Basic("throwing_executor") {}

    void enqueue(plain::concurrency::Task) override {
      throw executor_enqueue_exception();
    }

    void enqueue(std::span<plain::concurrency::Task>) override {
      throw executor_enqueue_exception();
    }

    int max_concurrency_level() const noexcept override {
      return 0;
    }

    bool shutdown_requested() const noexcept override {
      return false;
    }

    void shutdown() noexcept override {
      // do nothing
    }
  };
}  // namespace plain::tests

#endif  // PLAIN_CORE_TEST_RESULT_H_ELPERS_H_
