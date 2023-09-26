#ifndef PLAIN_CORE_TEST_EXECUTOR_TEST_H_ELPERS_H_
#define PLAIN_CORE_TEST_EXECUTOR_TEST_H_ELPERS_H_

#include "plain/concurrency/executor/basic.h"

namespace plain::tests {

struct executor_shutdowner {
  std::shared_ptr<plain::concurrency::executor::Basic> executor;

  executor_shutdowner(
    std::shared_ptr<plain::concurrency::executor::Basic> executor) noexcept :
    executor(std::move(executor)) {}

  ~executor_shutdowner() noexcept {
    executor->shutdown();
  }
};

}  // namespace plain::tests

#endif
