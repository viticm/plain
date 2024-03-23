#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/executor_shutdowner.h"

#include <chrono>

using namespace std::chrono_literals;

namespace plain::tests {

void test_timer_queue_make_timer();
void test_timer_queue_make_oneshot_timer();
void test_timer_queue_make_delay_object();
void test_timer_queue_max_worker_idle_time();
void test_timer_queue_thread_injection();
void test_timer_queue_thread_callbacks();

}  // namespace plain::tests

void plain::tests::test_timer_queue_make_timer() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  ASSERT_FALSE(timer_queue->shutdown_requested());

  assert_throws_with_error_message<std::invalid_argument>(
    [timer_queue] {
      timer_queue->make_timer(100ms, 100ms, {}, [] {
      });
    },
    "make_timer - executor is null.");

  timer_queue->shutdown();
  ASSERT_TRUE(timer_queue->shutdown_requested());

  assert_throws_with_error_message<std::runtime_error>(
    [timer_queue] {
      auto inline_executor = 
        std::make_shared<plain::concurrency::executor::Inline>();
      timer_queue->make_timer(100ms, 100ms, inline_executor, [] {
      });
    },
    "add_timer - has been shut down.");
}

void plain::tests::test_timer_queue_make_oneshot_timer() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  ASSERT_FALSE(timer_queue->shutdown_requested());

  assert_throws_with_error_message<std::invalid_argument>(
    [timer_queue] {
      timer_queue->make_one_shot_timer(100ms, {}, [] {
      });
    },
    "make_one_shot_timer - executor is null.");

  timer_queue->shutdown();
  ASSERT_TRUE(timer_queue->shutdown_requested());

  assert_throws_with_error_message<std::runtime_error>(
    [timer_queue] {
      auto inline_executor = 
        std::make_shared<plain::concurrency::executor::Inline>();
      timer_queue->make_one_shot_timer(100ms, inline_executor, [] {
      });
    },
    "add_timer - has been shut down.");
}

void plain::tests::test_timer_queue_make_delay_object() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  ASSERT_FALSE(timer_queue->shutdown_requested());

  assert_throws_with_error_message<std::invalid_argument>(
    [timer_queue] {
      timer_queue->make_delay_object(100ms, {});
    },
    "make_delay_object - executor is null.");

  timer_queue->shutdown();
  ASSERT_TRUE(timer_queue->shutdown_requested());

  assert_throws_with_error_message<std::runtime_error>(
    [timer_queue] {
      auto inline_executor = 
        std::make_shared<plain::concurrency::executor::Inline>();
      timer_queue->make_delay_object(100ms, inline_executor).run().get();
    },
    "associated task was interrupted abnormally.");
}

void plain::tests::test_timer_queue_max_worker_idle_time() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(1234567ms);
  ASSERT_EQ(timer_queue->max_worker_idle_time(), 1234567ms);
}

void plain::tests::test_timer_queue_thread_injection() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(50ms);
  object_observer observer;
  auto inline_executor = 
    std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner es(inline_executor);

  for (size_t i = 0; i < 5; i++) {
    auto timer = timer_queue->make_one_shot_timer(
      50ms, inline_executor, observer.get_testing_stub());
    std::this_thread::sleep_for(timer_queue->max_worker_idle_time() + 100ms);
  }

  const auto &execution_map = observer.get_execution_map();
  ASSERT_EQ(execution_map.size(), 5);
  for (const auto &count : execution_map) {
    ASSERT_EQ(count.second, 1);
  }
}

void plain::tests::test_timer_queue_thread_callbacks() {
  std::atomic_size_t thread_started_callback_invocations_num = 0;
  std::atomic_size_t thread_terminated_callback_invocations_num = 0;

  auto thread_started_callback = 
    [&thread_started_callback_invocations_num](std::string_view thread_name) {
    ++thread_started_callback_invocations_num;
    ASSERT_EQ(
      thread_name,
      plain::concurrency::executor::detail::make_executor_worker_name("timer queue"));
  };

  auto thread_terminated_callback = 
    [&thread_terminated_callback_invocations_num](std::string_view thread_name) {
    ++thread_terminated_callback_invocations_num;
    ASSERT_EQ(
      thread_name,
      plain::concurrency::executor::detail::make_executor_worker_name("timer queue"));
  };

  auto timer_queue = std::make_shared<plain::TimerQueue>(
    50ms, thread_started_callback, thread_terminated_callback);
  ASSERT_EQ(thread_started_callback_invocations_num, 0);
  ASSERT_EQ(thread_terminated_callback_invocations_num, 0);

  auto inline_executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner es(inline_executor);

  auto timer =
    timer_queue->make_one_shot_timer(
      50ms,
      inline_executor,
      [&thread_started_callback_invocations_num,
       &thread_terminated_callback_invocations_num]() {
        ASSERT_EQ(thread_started_callback_invocations_num, 1);
        ASSERT_EQ(thread_terminated_callback_invocations_num, 0);
      });

  timer_queue->shutdown();
  ASSERT_EQ(thread_started_callback_invocations_num, 1);
  ASSERT_EQ(thread_terminated_callback_invocations_num, 1);
}

using namespace plain::tests;

class TimerQueue : public testing::Test {

 public:
   static void SetUpTestCase() {
     //Normal.
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
   }

   virtual void TearDown() {
   }

};

TEST_F(TimerQueue, testMakeTimer) {
  test_timer_queue_make_timer();
}

TEST_F(TimerQueue, testMakeOneshotTimer) {
  test_timer_queue_make_oneshot_timer();
}

TEST_F(TimerQueue, testMakeDelayObject) {
  test_timer_queue_make_delay_object();
}

TEST_F(TimerQueue, testMaxWorkerIdleTime) {
  test_timer_queue_max_worker_idle_time();
}

TEST_F(TimerQueue, testThreadInjection) {
  test_timer_queue_thread_injection();
}

TEST_F(TimerQueue, testThreadCallbacks) {
  test_timer_queue_thread_callbacks();
}

/*
int main() {
  tester test("timer_queue test");

  test.add_step("make_timer", test_timer_queue_make_timer);
  test.add_step("make_oneshot_timer", test_timer_queue_make_timer);
  test.add_step("make_delay_object", test_timer_queue_make_delay_object);
  test.add_step("max_worker_idle_time", test_timer_queue_max_worker_idle_time);
  test.add_step("thread_injection", test_timer_queue_thread_injection);
  test.add_step("thread_callbacks", test_timer_queue_thread_callbacks);

  test.launch_test();
  return 0;
}
*/
