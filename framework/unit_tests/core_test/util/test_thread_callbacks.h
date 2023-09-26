#ifndef PLAIN_CORE_TEST_TEST_THREAD_CALLBACKS_H_
#define PLAIN_CORE_TEST_TEST_THREAD_CALLBACKS_H_

#include "assertions.h"
#include "util/executor_shutdowner.h"
#include "plain/concurrency/executor/basic.h"

namespace plain::tests {
  template<class executor_factory_type>
  void test_thread_callbacks(executor_factory_type executor_factory, std::string_view expected_thread_name) {
    std::atomic_size_t thread_started_callback_invocations_num = 0;
    std::atomic_size_t thread_terminated_callback_invocations_num = 0;

    auto thread_started_callback = [&thread_started_callback_invocations_num, expected_thread_name](std::string_view thread_name) {
      ++thread_started_callback_invocations_num;
      ASSERT_EQ(thread_name, expected_thread_name);
    };

    auto thread_terminated_callback = [&thread_terminated_callback_invocations_num,
                       expected_thread_name](std::string_view thread_name) {
      ++thread_terminated_callback_invocations_num;
      ASSERT_EQ(thread_name, expected_thread_name);
    };

    std::shared_ptr<concurrency::executor::Basic> executor =
      executor_factory(thread_started_callback, thread_terminated_callback);
    executor_shutdowner es(executor);

    ASSERT_EQ(thread_started_callback_invocations_num, 0);
    ASSERT_EQ(thread_terminated_callback_invocations_num, 0);

    executor
      ->submit([&thread_started_callback_invocations_num, &thread_terminated_callback_invocations_num]() {
        ASSERT_EQ(thread_started_callback_invocations_num, 1);
        ASSERT_EQ(thread_terminated_callback_invocations_num, 0);
      })
      .get();

    executor->shutdown();
    ASSERT_EQ(thread_started_callback_invocations_num, 1);
    ASSERT_EQ(thread_terminated_callback_invocations_num, 1);
  }
}  // namespace plain::tests

#endif
