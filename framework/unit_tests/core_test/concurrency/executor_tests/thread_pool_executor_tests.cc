#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "util/wait_context.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"
#include "util/test_thread_callbacks.h"

namespace plain::tests {

void test_thread_pool_executor_name();

void test_thread_pool_executor_shutdown_thread_join();
void test_thread_pool_executor_shutdown_method_access();
void test_thread_pool_executor_shutdown_method_more_than_once();
void test_thread_pool_executor_shutdown();

void test_thread_pool_executor_post_exception();
void test_thread_pool_executor_post_foreign();
void test_thread_pool_executor_post_inline();
void test_thread_pool_executor_post();

void test_thread_pool_executor_submit_exception();
void test_thread_pool_executor_submit_foreign();
void test_thread_pool_executor_submit_inline();
void test_thread_pool_executor_submit();

void test_thread_pool_executor_bulk_post_exception();
void test_thread_pool_executor_bulk_post_foreign();
void test_thread_pool_executor_bulk_post_inline();
void test_thread_pool_executor_bulk_post();

void test_thread_pool_executor_bulk_submit_exception();
void test_thread_pool_executor_bulk_submit_foreign();
void test_thread_pool_executor_bulk_submit_inline();
void test_thread_pool_executor_bulk_submit();

void test_thread_pool_executor_enqueue_algorithm();
void test_thread_pool_executor_dynamic_resizing();

void test_thread_pool_executor_thread_callbacks();

}  // namespace plain::tests


void plain::tests::test_thread_pool_executor_name() {
  const auto name = "abcde12345&*(";
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      name, 4, std::chrono::seconds(10));
  executor_shutdowner shutdowner(executor);
  ASSERT_EQ(executor->name_, name);
}

void plain::tests::test_thread_pool_executor_shutdown_thread_join() {
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 9, std::chrono::seconds(1));
  object_observer observer;

  for (size_t i = 0; i < 3; i++) {
    executor->post([stub = observer.get_testing_stub()]() mutable {
      stub();
    });
  }

  for (size_t i = 0; i < 3; i++) {
    executor->post([stub = observer.get_testing_stub()]() mutable {
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      stub();
    });
  }

  // allow threads time to start working
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // 1/3 of the threads are waiting, 1/3 are working, 1/3 are idle. all should
  // be joined when tp is shut-down.
  executor->shutdown();
  ASSERT_TRUE(executor->shutdown_requested());
  ASSERT_EQ(observer.get_execution_count(), 6);
  ASSERT_EQ(observer.get_destruction_count(), 6);
}

void plain::tests::test_thread_pool_executor_shutdown_method_access() {
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 4, std::chrono::seconds(10));
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

void plain::tests::test_thread_pool_executor_shutdown_method_more_than_once() {
  const size_t task_count = 64;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 4, std::chrono::seconds(10));

  for (size_t i = 0; i < task_count; i++) {
    executor->post([] {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
  }

  for (size_t i = 0; i < 4; i++) {
    executor->shutdown();
  }
}

void plain::tests::test_thread_pool_executor_shutdown() {
  test_thread_pool_executor_shutdown_thread_join();
  test_thread_pool_executor_shutdown_method_access();
  test_thread_pool_executor_shutdown_method_more_than_once();
}

void plain::tests::test_thread_pool_executor_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 1, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  executor->post([] {
    throw std::runtime_error("");
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void plain::tests::test_thread_pool_executor_post_foreign() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  for (size_t i = 0; i < task_count; i++) {
    executor->post(observer.get_testing_stub());
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_post_inline() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer, task_count] {
    for (size_t i = 0; i < task_count; i++) {
      executor->post(observer.get_testing_stub());
    }
  });

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_post() {
  test_thread_pool_executor_post_exception();
  test_thread_pool_executor_post_foreign();
  test_thread_pool_executor_post_inline();
}

void plain::tests::test_thread_pool_executor_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 4, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  constexpr intptr_t id = 12345;

  auto result = executor->submit([id] {
    throw custom_exception(id);
  });

  result.wait();
  test_ready_result_custom_exception(std::move(result), id);
}

void plain::tests::test_thread_pool_executor_submit_foreign() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  std::vector<concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_submit_inline() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  auto results_res = executor->submit([executor, &observer, task_count] {
    std::vector<concurrency::Result<size_t>> results;
    results.resize(task_count);
    for (size_t i = 0; i < task_count; i++) {
      results[i] = executor->submit(observer.get_testing_stub(i));
    }

    return results;
  });

  auto results = results_res.get();
  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), static_cast<size_t>(i));
  }

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_submit() {
  test_thread_pool_executor_submit_exception();
  test_thread_pool_executor_submit_foreign();
  test_thread_pool_executor_submit_inline();
}

void plain::tests::test_thread_pool_executor_bulk_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 4, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  auto thrower = [] {
    throw std::runtime_error("");
  };

  std::vector<decltype(thrower)> tasks;
  tasks.resize(4);

  executor->bulk_post<decltype(thrower)>(tasks);
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void plain::tests::test_thread_pool_executor_bulk_post_foreign() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  std::vector<testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub());
  }

  executor->bulk_post<testing_stub>(stubs);

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_bulk_post_inline() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer, task_count]() mutable {
    std::vector<testing_stub> stubs;
    stubs.reserve(task_count);

    for (size_t i = 0; i < task_count; i++) {
      stubs.emplace_back(observer.get_testing_stub());
    }

    executor->bulk_post<testing_stub>(stubs);
  });

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_bulk_post() {
  test_thread_pool_executor_bulk_post_exception();
  test_thread_pool_executor_bulk_post_foreign();
  test_thread_pool_executor_bulk_post_inline();
}

void plain::tests::test_thread_pool_executor_bulk_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 4, std::chrono::seconds(10));
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

void plain::tests::test_thread_pool_executor_bulk_submit_foreign() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
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

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_bulk_submit_inline() {
  const auto worker_count = thread::hardware_concurrency();
  const auto task_count = worker_count  *10'000;

  object_observer observer;
  auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
  executor_shutdowner shutdown(executor);

  auto results_res = executor->submit([executor, &observer, task_count] {
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

  ASSERT_TRUE(observer.wait_execution_count(task_count, std::chrono::minutes(2)));
  ASSERT_TRUE(observer.wait_destruction_count(task_count, std::chrono::minutes(2)));
}

void plain::tests::test_thread_pool_executor_bulk_submit() {
  test_thread_pool_executor_bulk_submit_exception();
  test_thread_pool_executor_bulk_submit_foreign();
  test_thread_pool_executor_bulk_submit_inline();
}

void plain::tests::test_thread_pool_executor_enqueue_algorithm() {
  // case 1 : if this is a thread pool worker thread and this worker seems to be empty (both publicly and privately)
  // enqueue to self
  {
    const size_t worker_count = 2;
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
    executor_shutdowner shutdown(executor);

    executor->post([executor, &observer] {
      observer.get_testing_stub()();

      executor->post([stub = observer.get_testing_stub()]() mutable {  // will enqueue it to self
        stub();
      });
    });

    observer.wait_destruction_count(2, std::chrono::seconds(20));

    ASSERT_EQ(observer.get_execution_map().size(), static_cast<size_t>(1));
  }

  // case 2 : if (1) is false =>  if an idle thread exists, enqueue it to the idle thread
  {
    object_observer observer;
    const size_t worker_count = 6;
    auto wc = std::make_shared<wait_context>();

    auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < worker_count; i++) {
      executor->post([wc, stub = observer.get_testing_stub()]() mutable {
        wc->wait();  // make sure this thread is not idle by imitating endless work
        stub();
      });
    }

    wc->notify();

    observer.wait_execution_count(worker_count, std::chrono::seconds(6));

    ASSERT_EQ(observer.get_execution_map().size(), worker_count);

    // make sure each task was executed on one thread - a task wasn't posted to a
    // non-idle thread.
    for (const auto &[thread_id, invocation_count] : observer.get_execution_map()) {
      ASSERT_EQ(invocation_count, static_cast<size_t>(1));
    }
  }

  // case 3 : if (2) is false => if this is a thread-pool thread, enqueue to
  // self
  {
    object_observer observer;
    auto wc = std::make_shared<wait_context>();

    auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", 2, std::chrono::seconds(10));
    executor_shutdowner shutdown(executor);

    executor->post([wc]() {
      wc->wait();
    });

    constexpr size_t task_count = 1'024;

    executor->post([&observer, executor] {
      for (size_t i = 0; i < task_count; i++) {
        executor->post(observer.get_testing_stub());
      }
    });

    observer.wait_execution_count(task_count, std::chrono::minutes(1));
    observer.wait_destruction_count(task_count, std::chrono::minutes(1));

    ASSERT_EQ(observer.get_execution_map().size(), static_cast<size_t>(1));

    wc->notify();
  }

  // case 3 : if (2) is false, choose a worker using round robin
  {
    const size_t task_count = 4'024;
    const size_t worker_count = 4;
    object_observer observer;
    auto wc = std::make_shared<wait_context>();

    auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(10));
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < worker_count; i++) {
      executor->post([wc]() {
        wc->wait();
      });
    }

    // now all workers are "busy" (blocked, in reality), tasks should be spread using round-robin
    for (size_t i = 0; i < task_count; i++) {
      executor->post(observer.get_testing_stub());
    }

    wc->notify();

    observer.wait_execution_count(task_count, std::chrono::minutes(1));
    observer.wait_destruction_count(task_count, std::chrono::minutes(1));

    ASSERT_EQ(observer.get_execution_map().size(), worker_count);
  }
}

void plain::tests::test_thread_pool_executor_dynamic_resizing() {
  // if workers are only waiting - notify them
  {
    const size_t worker_count = 4;
    const size_t iterations = 4;
    const size_t task_count = 1'024;
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, std::chrono::seconds(5));
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < iterations; i++) {
      for (size_t j = 0; j < task_count; j++) {
        executor->post(observer.get_testing_stub());
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(150));

      // in between, threads are waiting for an event (abort/task)
    }

    observer.wait_execution_count(task_count  *iterations, std::chrono::minutes(1));
    observer.wait_destruction_count(task_count  *iterations, std::chrono::minutes(1));

    // if all the tasks were launched by <<worker_count>> workers, then no new
    // workers were injected
    ASSERT_EQ(observer.get_execution_map().size(), worker_count);
  }

  // case 2 : if max_idle_time reached, idle threads exit. new threads are
  // injected when new tasks arrive
  {
    const size_t iterations = 4;
    const size_t worker_count = 4;
    const size_t task_count = 4'000;
    const auto max_idle_time = std::chrono::milliseconds(200);
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::ThreadPool>(
      "threadpool", worker_count, max_idle_time);
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < iterations; i++) {
      for (size_t j = 0; j < task_count; j++) {
        executor->post(observer.get_testing_stub());
      }

      std::this_thread::sleep_for(max_idle_time + std::chrono::milliseconds(150));
      // in between, threads are idling
    }

    observer.wait_execution_count(
      task_count  *iterations, std::chrono::minutes(1));
    observer.wait_execution_count(
      task_count  *iterations, std::chrono::minutes(1));

    /*
      If all the tasks were executed by <<worker_count>>  *iterations
      workers, then in every iteration a new set of threads was injected,
      meaning that the previous set of threads had exited.
    */
    ASSERT_EQ(observer.get_execution_map().size(), worker_count  *iterations);
  }
}

void plain::tests::test_thread_pool_executor_thread_callbacks() {
  constexpr std::string_view thread_pool_name = "threadpool";
  test_thread_callbacks(
    [thread_pool_name](auto started_callback, auto terminated_callback) {
      return std::make_shared<plain::concurrency::executor::ThreadPool>(
        thread_pool_name,
        1,
        std::chrono::seconds(10),
        started_callback,
        terminated_callback);
    },
    concurrency::executor::detail::make_executor_worker_name(thread_pool_name));
}

using namespace plain::tests;

class ThreadPoolExecutor : public testing::Test {

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

TEST_F(ThreadPoolExecutor, testName) {
  test_thread_pool_executor_name();
}

TEST_F(ThreadPoolExecutor, testShutdown) {
  test_thread_pool_executor_shutdown();
}

TEST_F(ThreadPoolExecutor, testPost) {
  test_thread_pool_executor_post();
}

TEST_F(ThreadPoolExecutor, testSubmit) {
  test_thread_pool_executor_submit();
}

TEST_F(ThreadPoolExecutor, testBulkPost) {
  test_thread_pool_executor_bulk_post();
}

TEST_F(ThreadPoolExecutor, testBulkSubmit) {
  test_thread_pool_executor_bulk_submit();
}

TEST_F(ThreadPoolExecutor, testEnqueuingAlgorithm) {
  test_thread_pool_executor_enqueue_algorithm();
}

TEST_F(ThreadPoolExecutor, testDynamicResizing) {
  test_thread_pool_executor_dynamic_resizing();
}

TEST_F(ThreadPoolExecutor, testThreadCallbacks) {
  test_thread_pool_executor_thread_callbacks();
}

/*
int main() {
  tester tester("thread_pool_executor test");

  tester.add_step("name", test_thread_pool_executor_name);
  tester.add_step("shutdown", test_thread_pool_executor_shutdown);
  tester.add_step("post", test_thread_pool_executor_post);
  tester.add_step("submit", test_thread_pool_executor_submit);
  tester.add_step("bulk_post", test_thread_pool_executor_bulk_post);
  tester.add_step("bulk_submit", test_thread_pool_executor_bulk_submit);
  tester.add_step("enqueuing algorithm", test_thread_pool_executor_enqueue_algorithm);
  tester.add_step("dynamic resizing", test_thread_pool_executor_dynamic_resizing);
  tester.add_step("thread_callbacks", test_thread_pool_executor_thread_callbacks);

  tester.launch_test();
  return 0;
}
*/
