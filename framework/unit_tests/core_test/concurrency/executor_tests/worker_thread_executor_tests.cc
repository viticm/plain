#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"
#include "util/test_thread_callbacks.h"

namespace plain::tests {

void test_worker_thread_executor_name();

void test_worker_thread_executor_shutdown_method_access();
void test_worker_thread_executor_shutdown_thread_join();
void test_worker_thread_executor_shutdown_more_than_once();
void test_worker_thread_executor_shutdown();

void test_worker_thread_executor_max_concurrency_level();

void test_worker_thread_executor_post_exception();
void test_worker_thread_executor_post_foreign();
void test_worker_thread_executor_post_inline();
void test_worker_thread_executor_post();

void test_worker_thread_executor_submit_exception();
void test_worker_thread_executor_submit_foreign();
void test_worker_thread_executor_submit_inline();
void test_worker_thread_executor_submit();

void test_worker_thread_executor_bulk_post_exception();
void test_worker_thread_executor_bulk_post_foreign();
void test_worker_thread_executor_bulk_post_inline();
void test_worker_thread_executor_bulk_post();

void test_worker_thread_executor_bulk_submit_exception();
void test_worker_thread_executor_bulk_submit_foreign();
void test_worker_thread_executor_bulk_submit_inline();
void test_worker_thread_executor_bulk_submit();

void test_worker_thread_executor_thread_callbacks();

void assert_unique_execution_thread(
  const std::unordered_map<size_t, size_t> &execution_map) {
  ASSERT_EQ(execution_map.size(), 1);
  ASSERT_NE(execution_map.begin()->first, plain::thread::get_current_virtual_id());
}

}  // namespace plain::tests


void plain::tests::test_worker_thread_executor_name() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->name_, "worker thread");
}

void plain::tests::test_worker_thread_executor_shutdown_method_access() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  ASSERT_FALSE(executor->shutdown_requested());

  executor->shutdown();
  ASSERT_TRUE(executor->shutdown_requested());

  assert_throws<std::runtime_error>([executor] {
    executor->enqueue(plain::concurrency::Task {});
  });

  assert_throws<std::runtime_error>([executor] {
    plain::concurrency::Task array[4];
    std::span<plain::concurrency::Task> span = array;
    executor->enqueue(span);
  });
}

void plain::tests::test_worker_thread_executor_shutdown_thread_join() {
  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();

  executor->post([stub = observer.get_testing_stub()]() mutable {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stub();
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(30));

  executor->shutdown();
  ASSERT_TRUE(executor->shutdown_requested());
  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(1));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(1));
}

void plain::tests::test_worker_thread_executor_shutdown_more_than_once() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  for (size_t i = 0; i < 4; i++) {
    executor->shutdown();
  }
}

void plain::tests::test_worker_thread_executor_shutdown() {
  test_worker_thread_executor_shutdown_method_access();
  test_worker_thread_executor_shutdown_thread_join();
  test_worker_thread_executor_shutdown_more_than_once();
}

void plain::tests::test_worker_thread_executor_max_concurrency_level() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->max_concurrency_level(), 1);
}

void plain::tests::test_worker_thread_executor_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  executor->post([] {
    throw std::runtime_error("");
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void plain::tests::test_worker_thread_executor_post_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  for (size_t i = 0; i < task_count; i++) {
    executor->post(observer.get_testing_stub());
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_post_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer] {
    for (size_t i = 0; i < task_count; i++) {
      executor->post(observer.get_testing_stub());
    }
  });

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_post() {
  test_worker_thread_executor_post_exception();
  test_worker_thread_executor_post_foreign();
  test_worker_thread_executor_post_inline();
}

void plain::tests::test_worker_thread_executor_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);
  constexpr intptr_t id = 12345;

  auto result = executor->submit([id] {
    throw custom_exception(id);
  });

  result.wait();
  test_ready_result_custom_exception(std::move(result), id);
}

void plain::tests::test_worker_thread_executor_submit_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  std::vector<plain::concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_submit_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  auto results_res = executor->submit([executor, &observer] {
    std::vector<plain::concurrency::Result<size_t>> results;
    results.resize(task_count);
    for (size_t i = 0; i < task_count; i++) {
      results[i] = executor->submit(observer.get_testing_stub(i));
    }

    return results;
  });

  auto results = results_res.get();
  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_submit() {
  test_worker_thread_executor_submit_exception();
  test_worker_thread_executor_submit_foreign();
  test_worker_thread_executor_submit_inline();
}

void plain::tests::test_worker_thread_executor_bulk_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  auto thrower = [] {
    throw std::runtime_error("");
  };

  std::vector<decltype(thrower)> tasks;
  tasks.resize(4);

  executor->bulk_post<decltype(thrower)>(tasks);

  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void plain::tests::test_worker_thread_executor_bulk_post_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  std::vector<testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub());
  }

  executor->bulk_post<testing_stub>(stubs);

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_bulk_post_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer]() mutable {
    std::vector<testing_stub> stubs;
    stubs.reserve(task_count);

    for (size_t i = 0; i < task_count; i++) {
      stubs.emplace_back(observer.get_testing_stub());
    }

    executor->bulk_post<testing_stub>(stubs);
  });

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_bulk_post() {
  test_worker_thread_executor_bulk_post_exception();
  test_worker_thread_executor_bulk_post_foreign();
  test_worker_thread_executor_bulk_post_inline();
}

void plain::tests::test_worker_thread_executor_bulk_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);
  constexpr intptr_t id = 12345;

  auto thrower = [] {
    throw custom_exception(id);
  };

  std::vector<decltype(thrower)> tasks;
  tasks.resize(4, thrower);

  auto results = executor->bulk_submit<decltype(thrower)>(tasks);

  for (auto &result : results) {
    result.wait();
    test_ready_result_custom_exception(std::move(result), id);
  }
}

void plain::tests::test_worker_thread_executor_bulk_submit_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  std::vector<value_testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub(i));
  }

  auto results = executor->bulk_submit<value_testing_stub>(stubs);
  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_bulk_submit_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown(executor);

  auto results_res = executor->submit([executor, &observer] {
    std::vector<value_testing_stub> stubs;
    stubs.reserve(task_count);

    for (size_t i = 0; i < task_count; i++) {
      stubs.emplace_back(observer.get_testing_stub(i));
    }

    return executor->bulk_submit<value_testing_stub>(stubs);
  });

  auto results = results_res.get();
  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(1)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(1)));
  assert_unique_execution_thread(observer.get_execution_map());
}

void plain::tests::test_worker_thread_executor_bulk_submit() {
  test_worker_thread_executor_bulk_submit_exception();
  test_worker_thread_executor_bulk_submit_foreign();
  test_worker_thread_executor_bulk_submit_inline();
}

void plain::tests::test_worker_thread_executor_thread_callbacks() {
  test_thread_callbacks(
    [](auto thread_started_callback, auto thread_terminated_callback) {
      return std::make_shared<plain::concurrency::executor::WorkerThread>(
        thread_started_callback, thread_terminated_callback);
    },
    plain::concurrency::executor::detail::make_executor_worker_name("worker thread"));
}

using namespace plain::tests;

class WorkerThreadExecutor : public testing::Test {

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

TEST_F(WorkerThreadExecutor, testName) {
  test_worker_thread_executor_name();
}

TEST_F(WorkerThreadExecutor, testShutdown) {
  test_worker_thread_executor_shutdown();
}

TEST_F(WorkerThreadExecutor, testMaxConcurrencyLevel) {
  test_worker_thread_executor_max_concurrency_level();
}

TEST_F(WorkerThreadExecutor, testPost) {
  test_worker_thread_executor_post();
}

TEST_F(WorkerThreadExecutor, testSubmit) {
  test_worker_thread_executor_submit();
}

TEST_F(WorkerThreadExecutor, testBulkPost) {
  test_worker_thread_executor_bulk_post();
}

TEST_F(WorkerThreadExecutor, testBulkSubmit) {
  test_worker_thread_executor_bulk_submit();
}

TEST_F(WorkerThreadExecutor, testThreadCallbacks) {
  test_worker_thread_executor_thread_callbacks();
}

/*
int main() {
  tester tester("worker_thread_executor test");

  tester.add_step("name", test_worker_thread_executor_name);
  tester.add_step("shutdown", test_worker_thread_executor_shutdown);
  tester.add_step("max_concurrency_level", test_worker_thread_executor_max_concurrency_level);
  tester.add_step("post", test_worker_thread_executor_post);
  tester.add_step("submit", test_worker_thread_executor_submit);
  tester.add_step("bulk_post", test_worker_thread_executor_bulk_post);
  tester.add_step("bulk_submit", test_worker_thread_executor_bulk_submit);
  tester.add_step("thread_callbacks", test_worker_thread_executor_thread_callbacks);

  tester.launch_test();
  return 0;
}
*/
