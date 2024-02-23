#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"

namespace plain::tests {
  void test_inline_executor_name();

  void test_inline_executor_shutdown();

  void test_inline_executor_max_concurrency_level();

  void test_inline_executor_post_exception();
  void test_inline_executor_post_foreign();
  void test_inline_executor_post_inline();
  void test_inline_executor_post();

  void test_inline_executor_submit_exception();
  void test_inline_executor_submit_foreign();
  void test_inline_executor_submit_inline();
  void test_inline_executor_submit();

  void test_inline_executor_bulk_post_exception();
  void test_inline_executor_bulk_post_foreign();
  void test_inline_executor_bulk_post_inline();
  void test_inline_executor_bulk_post();

  void test_inline_executor_bulk_submit_exception();
  void test_inline_executor_bulk_submit_foreign();
  void test_inline_executor_bulk_submit_inline();
  void test_inline_executor_bulk_submit();

  void assert_executed_inline(
    const std::unordered_map<size_t, size_t> &execution_map) noexcept {
    ASSERT_EQ(execution_map.size(), static_cast<size_t>(1));
    ASSERT_EQ(
      execution_map.begin()->first, ::plain::thread::get_current_virtual_id());
  }
}  // namespace plain::tests

void plain::tests::test_inline_executor_name() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->name_, "inline");
}

void plain::tests::test_inline_executor_shutdown() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  ASSERT_FALSE(executor->shutdown_requested());

  executor->shutdown();
  ASSERT_TRUE(executor->shutdown_requested());

  // it's ok to shut down an executor more than once
  executor->shutdown();

  assert_throws<std::runtime_error>([executor] {
    executor->enqueue(plain::concurrency::Task {});
  });

  assert_throws<std::runtime_error>([executor] {
    plain::concurrency::Task array[4];
    std::span<plain::concurrency::Task> span = array;
    executor->enqueue(span);
  });
}

void plain::tests::test_inline_executor_max_concurrency_level() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->max_concurrency_level(), 0);
}

void plain::tests::test_inline_executor_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  executor->post([] {
    throw std::runtime_error("");
  });
}

void plain::tests::test_inline_executor_post_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  for (size_t i = 0; i < task_count; i++) {
    executor->post(observer.get_testing_stub());
  }

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_post_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer] {
    for (size_t i = 0; i < task_count; i++) {
      executor->post(observer.get_testing_stub());
    }
  });

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_post() {
  test_inline_executor_post_exception();
  test_inline_executor_post_inline();
  test_inline_executor_post_foreign();
}

void plain::tests::test_inline_executor_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);
  constexpr intptr_t id = 12345;

  auto result = executor->submit([id] {
    throw custom_exception(id);
  });

  result.wait();
  test_ready_result_custom_exception(std::move(result), id);
}

void plain::tests::test_inline_executor_submit_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  std::vector<plain::concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].status(), plain::concurrency::ResultStatus::Value);
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_submit_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
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
    ASSERT_EQ(results[i].status(), plain::concurrency::ResultStatus::Value);
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_submit() {
  test_inline_executor_submit_exception();
  test_inline_executor_submit_foreign();
  test_inline_executor_submit_inline();
}

void plain::tests::test_inline_executor_bulk_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  auto thrower = [] {
    throw std::runtime_error("");
  };

  std::vector<decltype(thrower)> tasks;
  tasks.resize(4);

  executor->bulk_post<decltype(thrower)>(tasks);
}

void plain::tests::test_inline_executor_bulk_post_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  std::vector<testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub());
  }

  std::span<testing_stub> span = stubs;
  executor->bulk_post<testing_stub>(span);

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_bulk_post_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer]() mutable {
    std::vector<testing_stub> stubs;
    stubs.reserve(task_count);

    for (size_t i = 0; i < task_count; i++) {
      stubs.emplace_back(observer.get_testing_stub());
    }

    executor->bulk_post<testing_stub>(stubs);
  });

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_bulk_post() {
  test_inline_executor_bulk_post_exception();
  test_inline_executor_bulk_post_foreign();
  test_inline_executor_bulk_post_inline();
}

void plain::tests::test_inline_executor_bulk_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
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

void plain::tests::test_inline_executor_bulk_submit_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  executor_shutdowner shutdown(executor);

  std::vector<value_testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub(i));
  }

  auto results = executor->bulk_submit<value_testing_stub>(stubs);
  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].status(), plain::concurrency::ResultStatus::Value);
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_bulk_submit_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
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
    ASSERT_EQ(results[i].status(), plain::concurrency::ResultStatus::Value);
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_EQ(observer.get_execution_count(), task_count);
  ASSERT_EQ(observer.get_destruction_count(), task_count);
  assert_executed_inline(observer.get_execution_map());
}

void plain::tests::test_inline_executor_bulk_submit() {
  test_inline_executor_bulk_submit_exception();
  test_inline_executor_bulk_submit_foreign();
  test_inline_executor_bulk_submit_inline();
}

using namespace plain::tests;

class InlineExecutor : public testing::Test {

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

TEST_F(InlineExecutor, testName) {
  test_inline_executor_name();
}

TEST_F(InlineExecutor, testShutdown) {
  test_inline_executor_shutdown();
}

TEST_F(InlineExecutor, testMaxConcurrencyLevel) {
  test_inline_executor_max_concurrency_level();
}

TEST_F(InlineExecutor, testPost) {
  test_inline_executor_post();
}

TEST_F(InlineExecutor, testSubmit) {
  test_inline_executor_submit();
}

TEST_F(InlineExecutor, testBulkPost) {
  test_inline_executor_bulk_post();
}

TEST_F(InlineExecutor, testBulkSubmit) {
  test_inline_executor_bulk_submit();
}
