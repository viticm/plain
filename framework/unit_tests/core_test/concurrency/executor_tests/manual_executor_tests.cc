#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"

namespace plain::tests {

void test_manual_executor_name();

void test_manual_executor_shutdown_method_access();
void test_manual_executor_shutdown_more_than_once();
void test_manual_executor_shutdown();

void test_manual_executor_max_concurrency_level();

void test_manual_executor_post_exception();
void test_manual_executor_post_foreign();
void test_manual_executor_post_inline();
void test_manual_executor_post();

void test_manual_executor_submit_exception();
void test_manual_executor_submit_foreign();
void test_manual_executor_submit_inline();
void test_manual_executor_submit();

void test_manual_executor_bulk_post_exception();
void test_manual_executor_bulk_post_foreign();
void test_manual_executor_bulk_post_inline();
void test_manual_executor_bulk_post();

void test_manual_executor_bulk_submit_exception();
void test_manual_executor_bulk_submit_foreign();
void test_manual_executor_bulk_submit_inline();
void test_manual_executor_bulk_submit();

void test_manual_executor_loop_once();
void test_manual_executor_loop_once_for();
void test_manual_executor_loop_once_until();

void test_manual_executor_loop();
void test_manual_executor_loop_for();
void test_manual_executor_loop_until();

void test_manual_executor_clear();

void test_manual_executor_wait_for_task();
void test_manual_executor_wait_for_task_for();
void test_manual_executor_wait_for_task_until();

void test_manual_executor_wait_for_tasks();
void test_manual_executor_wait_for_tasks_for();
void test_manual_executor_wait_for_tasks_until();

void assert_executed_locally(
  const std::unordered_map<size_t, size_t> &execution_map) {
  ASSERT_EQ(execution_map.size(), static_cast<size_t>(1));  // only one thread executed the tasks
  ASSERT_EQ(
    execution_map.begin()->first, plain::thread::get_current_virtual_id());  // and it's this thread.
}

}  // namespace plain::tests

using namespace std::chrono;

void plain::tests::test_manual_executor_name() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  ASSERT_EQ(executor->name_, "manual");
}

void plain::tests::test_manual_executor_shutdown_method_access() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
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

  assert_throws<std::runtime_error>([executor] {
    executor->clear();
  });

  assert_throws<std::runtime_error>([executor] {
    executor->loop_once();
  });

  assert_throws<std::runtime_error>([executor] {
    executor->loop_once_for(milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->loop_once_until(
      high_resolution_clock::now() + milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->loop(100);
  });

  assert_throws<std::runtime_error>([executor] {
    executor->loop_for(100, milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->loop_until(
      100, high_resolution_clock::now() + milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->wait_for_task();
  });

  assert_throws<std::runtime_error>([executor] {
    executor->wait_for_task_for(milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->wait_for_task_until(
      high_resolution_clock::now() + milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->wait_for_tasks(8);
  });

  assert_throws<std::runtime_error>([executor] {
    executor->wait_for_tasks_for(8, milliseconds(100));
  });

  assert_throws<std::runtime_error>([executor] {
    executor->wait_for_tasks_until(
      8, high_resolution_clock::now() + milliseconds(100));
  });
}

void plain::tests::test_manual_executor_shutdown_more_than_once() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  for (size_t i = 0; i < 4; i++) {
    executor->shutdown();
  }
}

void plain::tests::test_manual_executor_shutdown() {
  test_manual_executor_shutdown_method_access();
  test_manual_executor_shutdown_more_than_once();
}

void plain::tests::test_manual_executor_max_concurrency_level() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->max_concurrency_level(), std::numeric_limits<int>::max());
}

void plain::tests::test_manual_executor_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  executor->post([] {
    throw std::runtime_error("");
  });

  ASSERT_TRUE(executor->loop_once());
}

void plain::tests::test_manual_executor_post_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();

  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_TRUE(executor->empty());

  for (size_t i = 0; i < task_count; i++) {
    executor->post(observer.get_testing_stub());
    ASSERT_EQ(executor->size(), 1 + i);
    ASSERT_FALSE(executor->empty());
  }

  // manual executor doesn't execute the tasks automatically, hence manual.
  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(observer.get_execution_count(), i + 1);
  }

  executor->shutdown();
  ASSERT_EQ(observer.get_destruction_count(), task_count);

  assert_executed_locally(observer.get_execution_map());
}

void plain::tests::test_manual_executor_post_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();

  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_TRUE(executor->empty());

  executor->post([executor, &observer] {
    for (size_t i = 0; i < task_count; i++) {
      executor->post(observer.get_testing_stub());
      ASSERT_EQ(executor->size(), 1 + i);
      ASSERT_FALSE(executor->empty());
    }
  });

  // the tasks are not enqueued yet, only the spawning task is.
  ASSERT_EQ(executor->size(), static_cast<size_t>(1));
  ASSERT_FALSE(executor->empty());

  ASSERT_TRUE(executor->loop_once());
  ASSERT_EQ(executor->size(), task_count);
  ASSERT_FALSE(executor->empty());

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();
  ASSERT_EQ(observer.get_destruction_count(), task_count);

  assert_executed_locally(observer.get_execution_map());
}

void plain::tests::test_manual_executor_post() {
  test_manual_executor_post_exception();
  test_manual_executor_post_foreign();
  test_manual_executor_post_inline();
}

void plain::tests::test_manual_executor_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdowner(executor);

  constexpr intptr_t id = 12345;

  auto result = executor->submit([id] {
    throw custom_exception(id);
  });

  ASSERT_TRUE(executor->loop_once());

  result.wait();
  test_ready_result_custom_exception(std::move(result), id);
}

void plain::tests::test_manual_executor_submit_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();

  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_TRUE(executor->empty());

  std::vector<concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(results[i].get(), i);
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();

  assert_executed_locally(observer.get_execution_map());

  for (size_t i = task_count / 2; i < task_count; i++) {
    assert_throws<std::runtime_error>([&, i] {
      results[i].get();
    });
  }

  ASSERT_EQ(observer.get_destruction_count(), task_count);
}

void plain::tests::test_manual_executor_submit_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();

  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_TRUE(executor->empty());

  auto results_res = executor->submit([executor, &observer] {
    std::vector<concurrency::Result<size_t>> results;
    results.resize(task_count);
    for (size_t i = 0; i < task_count; i++) {
      results[i] = executor->submit(observer.get_testing_stub(i));
    }

    return results;
  });

  // the tasks are not enqueued yet, only the spawning task is.
  ASSERT_EQ(executor->size(), static_cast<size_t>(1));
  ASSERT_FALSE(executor->empty());

  ASSERT_TRUE(executor->loop_once());
  ASSERT_EQ(executor->size(), task_count);
  ASSERT_FALSE(executor->empty());

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  ASSERT_EQ(results_res.status(), concurrency::ResultStatus::Value);
  auto results = results_res.get();

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(results[i].get(), i);
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();

  assert_executed_locally(observer.get_execution_map());

  for (size_t i = task_count / 2; i < task_count; i++) {
    assert_throws<std::runtime_error>([&, i] {
      results[i].get();
    });
  }

  ASSERT_EQ(observer.get_destruction_count(), task_count);
}

void plain::tests::test_manual_executor_submit() {
  test_manual_executor_submit_exception();
  test_manual_executor_submit_foreign();
  test_manual_executor_submit_inline();
}

void plain::tests::test_manual_executor_bulk_post_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  auto thrower = [] {
    throw std::runtime_error("");
  };

  std::vector<decltype(thrower)> tasks;
  tasks.resize(4);

  executor->bulk_post<decltype(thrower)>(tasks);
  ASSERT_EQ(executor->loop(4), 4);
}

void plain::tests::test_manual_executor_bulk_post_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();

  std::vector<testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub());
  }

  executor->bulk_post<testing_stub>(stubs);

  ASSERT_EQ(executor->size(), task_count);
  ASSERT_FALSE(executor->empty());

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();
  ASSERT_EQ(observer.get_destruction_count(), task_count);

  assert_executed_locally(observer.get_execution_map());
}

void plain::tests::test_manual_executor_bulk_post_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  executor->post([executor, &observer]() mutable {
    std::vector<testing_stub> stubs;
    stubs.reserve(task_count);

    for (size_t i = 0; i < task_count; i++) {
      stubs.emplace_back(observer.get_testing_stub());
    }

    executor->bulk_post<testing_stub>(stubs);
  });

  // the tasks are not enqueued yet, only the spawning task is.
  ASSERT_EQ(executor->size(), static_cast<size_t>(1));
  ASSERT_FALSE(executor->empty());

  ASSERT_TRUE(executor->loop_once());
  ASSERT_EQ(executor->size(), task_count);
  ASSERT_FALSE(executor->empty());

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();
  ASSERT_EQ(observer.get_destruction_count(), task_count);

  assert_executed_locally(observer.get_execution_map());
}

void plain::tests::test_manual_executor_bulk_post() {
  test_manual_executor_bulk_post_exception();
  test_manual_executor_bulk_post_foreign();
  test_manual_executor_bulk_post_inline();
}

void plain::tests::test_manual_executor_bulk_submit_exception() {
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);
  constexpr intptr_t id = 12345;

  auto thrower = [] {
    throw custom_exception(id);
  };

  std::vector<decltype(thrower)> tasks;
  tasks.resize(4, thrower);

  auto results = executor->bulk_submit<decltype(thrower)>(tasks);
  ASSERT_EQ(executor->loop(4), 4);

  for (auto &result : results) {
    result.wait();
    test_ready_result_custom_exception(std::move(result), id);
  }
}

void plain::tests::test_manual_executor_bulk_submit_foreign() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  std::vector<value_testing_stub> stubs;
  stubs.reserve(task_count);

  for (size_t i = 0; i < task_count; i++) {
    stubs.emplace_back(observer.get_testing_stub(i));
  }

  auto results = executor->bulk_submit<value_testing_stub>(stubs);

  ASSERT_FALSE(executor->empty());
  ASSERT_EQ(executor->size(), task_count);

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(results[i].get(), i);
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();

  assert_executed_locally(observer.get_execution_map());

  for (size_t i = task_count / 2; i < task_count; i++) {
    assert_throws<std::runtime_error>([&, i] {
      results[i].get();
    });
  }

  ASSERT_EQ(observer.get_destruction_count(), task_count);
}

void plain::tests::test_manual_executor_bulk_submit_inline() {
  object_observer observer;
  constexpr size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  auto results_res = executor->submit([executor, &observer] {
    std::vector<value_testing_stub> stubs;
    stubs.reserve(task_count);

    for (size_t i = 0; i < task_count; i++) {
      stubs.emplace_back(observer.get_testing_stub(i));
    }

    return executor->bulk_submit<value_testing_stub>(stubs);
  });

  // the tasks are not enqueued yet, only the spawning task is.
  ASSERT_EQ(executor->size(), static_cast<size_t>(1));
  ASSERT_FALSE(executor->empty());

  ASSERT_TRUE(executor->loop_once());
  ASSERT_EQ(executor->size(), task_count);
  ASSERT_FALSE(executor->empty());

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  ASSERT_EQ(results_res.status(), concurrency::ResultStatus::Value);
  auto results = results_res.get();

  for (size_t i = 0; i < task_count / 2; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(results[i].get(), i);
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
  }

  executor->shutdown();

  assert_executed_locally(observer.get_execution_map());

  for (size_t i = task_count / 2; i < task_count; i++) {
    assert_throws<std::runtime_error>([&, i] {
      results[i].get();
    });
  }

  ASSERT_EQ(observer.get_destruction_count(), task_count);
}

void plain::tests::test_manual_executor_bulk_submit() {
  test_manual_executor_bulk_submit_exception();
  test_manual_executor_bulk_submit_foreign();
  test_manual_executor_bulk_submit_inline();
}

void plain::tests::test_manual_executor_loop_once() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_TRUE(executor->empty());

  for (size_t i = 0; i < 10; i++) {
    ASSERT_FALSE(executor->loop_once());
  }

  std::vector<concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  for (size_t i = 0; i < task_count; i++) {
    ASSERT_TRUE(executor->loop_once());
    ASSERT_EQ(results[i].get(), i);
    ASSERT_EQ(observer.get_execution_count(), i + 1);
    ASSERT_EQ(observer.get_destruction_count(), i + 1);
    ASSERT_EQ(executor->size(), task_count - (i + 1));
  }

  assert_executed_locally(observer.get_execution_map());

  for (size_t i = 0; i < 10; i++) {
    ASSERT_FALSE(executor->loop_once());
  }
}

void plain::tests::test_manual_executor_loop_once_for() {
  // case 1: timeout
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    const auto waiting_time_ms = milliseconds(50);
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < 10; i++) {
      const auto before = high_resolution_clock::now();
      ASSERT_FALSE(executor->loop_once_for(waiting_time_ms));
      const auto after = high_resolution_clock::now();
      const auto ms_elapsed =
        std::chrono::duration_cast<milliseconds>(after - before);
      ASSERT_GE(ms_elapsed, waiting_time_ms);
    }
  }

  // case 2: tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    const auto waiting_time_ms = milliseconds(150);
    executor_shutdowner shutdown(executor);
    object_observer observer;

    executor->post(observer.get_testing_stub());
    const auto before = high_resolution_clock::now();
    ASSERT_TRUE(executor->loop_once_for(waiting_time_ms));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
    ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(1));
    ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(1));
    assert_executed_locally(observer.get_execution_map());
  }

  // case 3: goes to sleep, then woken by an incoming task
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    object_observer observer;
    const auto enqueue_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, enqueue_time, &observer]() mutable {
      std::this_thread::sleep_until(enqueue_time);
      executor->post(observer.get_testing_stub());
    });

    ASSERT_TRUE(executor->loop_once_for(seconds(10)));
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, enqueue_time);
    ASSERT_LE(now, enqueue_time + seconds(1));

    thread.join();

    assert_executed_locally(observer.get_execution_map());
  }

  // case 4: goes to sleep, then woken by a shutdown interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->loop_once_for(seconds(10));
    });

    const auto now = high_resolution_clock::now();
    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_loop_once_until() {
  // case 1: timeout
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < 10; i++) {
      const auto max_waiting_time_point =
        high_resolution_clock::now() + milliseconds(50);
      const auto before = high_resolution_clock::now();
      ASSERT_FALSE(executor->loop_once_until(max_waiting_time_point));
      const auto now = high_resolution_clock::now();
      ASSERT_GE(now, before);
      ASSERT_GE(now, max_waiting_time_point);
    }
  }

  // case 2: tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    object_observer observer;

    const auto max_waiting_time_point =
      high_resolution_clock::now() + milliseconds(150);
    executor->post(observer.get_testing_stub());
    const auto before = high_resolution_clock::now();
    ASSERT_TRUE(executor->loop_once_until(max_waiting_time_point));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
    ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(1));
    ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(1));
    assert_executed_locally(observer.get_execution_map());
  }

  // case 3: goes to sleep, then woken by an incoming task
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    object_observer observer;
    const auto enqueue_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, enqueue_time, &observer]() mutable {
      std::this_thread::sleep_until(enqueue_time);
      executor->post(observer.get_testing_stub());
    });

    const auto max_looping_time_point =
      high_resolution_clock::now() + seconds(10);
    ASSERT_TRUE(executor->loop_once_until(max_looping_time_point));
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, enqueue_time);
    ASSERT_LE(now, enqueue_time + seconds(1));

    thread.join();

    assert_executed_locally(observer.get_execution_map());
  }

  // case 4: goes to sleep, then woken by a shutdown interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      const auto max_looping_time_point =
        high_resolution_clock::now() + seconds(10);
      executor->loop_once_until(max_looping_time_point);
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_loop() {
  object_observer observer;
  const size_t task_count = 1'024;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_TRUE(executor->empty());

  ASSERT_EQ(executor->loop(100), static_cast<size_t>(0));

  std::vector<concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));

  const size_t chunk_size = 150;
  const auto cycles = task_count / chunk_size;
  const auto remained = task_count % (cycles  *chunk_size);

  for (size_t i = 0; i < cycles; i++) {
    const auto executed = executor->loop(chunk_size);
    ASSERT_EQ(executed, chunk_size);

    const auto total_executed = (i + 1)  *chunk_size;
    ASSERT_EQ(observer.get_execution_count(), total_executed);
    ASSERT_EQ(executor->size(), task_count - total_executed);
  }

  // execute the remaining tasks
  const auto executed = executor->loop(chunk_size);
  ASSERT_EQ(executed, remained);

  ASSERT_EQ(observer.get_execution_count(), task_count);

  ASSERT_TRUE(executor->empty());
  ASSERT_EQ(executor->size(), static_cast<size_t>(0));

  ASSERT_EQ(executor->loop(100), static_cast<size_t>(0));

  assert_executed_locally(observer.get_execution_map());

  for (size_t i = 0; i < task_count; i++) {
    ASSERT_EQ(results[i].get(), i);
  }

  ASSERT_EQ(observer.get_destruction_count(), task_count);
}

void plain::tests::test_manual_executor_loop_for() {
  // when max_count == 0, the function returns immediately
  {
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto before = high_resolution_clock::now();
    const auto executed = executor->loop_for(0, seconds(10));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = duration_cast<milliseconds>(after - before);
    ASSERT_EQ(executed, static_cast<size_t>(0));
    ASSERT_LE(ms_elapsed, milliseconds(5));
  }

  // when max_waiting_time == 0ms, the function behaves like manual_executor::loop
  {
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    ASSERT_EQ(executor->loop_for(100, milliseconds(0)), static_cast<size_t>(0));

    const size_t task_count = 100;
    for (size_t i = 0; i < task_count; i++) {
      executor->post(observer.get_testing_stub());
    }

    for (size_t i = 0; i < (task_count - 2) / 2; i++) {
      const auto executed = executor->loop_for(2, milliseconds(0));
      ASSERT_EQ(executed, 2);
      ASSERT_EQ(observer.get_execution_count(), (i + 1)  *2);
      ASSERT_EQ(observer.get_destruction_count(), (i + 1)  *2);
    }

    ASSERT_EQ(executor->loop_for(10, milliseconds(0)), 2);
    ASSERT_EQ(observer.get_execution_count(), 100);
    ASSERT_EQ(observer.get_destruction_count(), 100);

    ASSERT_EQ(executor->loop_for(10, milliseconds(0)), 0);

    assert_executed_locally(observer.get_execution_map());
  }

  // if max_count is reached, the function returns
  {
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto max_looping_time = seconds(10);
    const auto enqueueing_interval = milliseconds(100);
    const size_t max_count = 8;

    std::thread enqueuer([max_count, enqueueing_interval, executor, &observer] {
      for (size_t i = 0; i < max_count + 1; i++) {
        std::this_thread::sleep_for(enqueueing_interval);
        executor->post(observer.get_testing_stub());
      }
    });

    const auto before = high_resolution_clock::now();
    const auto executed = executor->loop_for(max_count, max_looping_time);
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = duration_cast<milliseconds>(after - before);

    ASSERT_EQ(executed, max_count);
    ASSERT_GE(ms_elapsed, max_count  *enqueueing_interval);
    ASSERT_LT(ms_elapsed, max_count  *enqueueing_interval + seconds(1));

    enqueuer.join();
  }

  // if shutdown requested, the function returns and throws
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->loop_for(100, seconds(10));
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_loop_until() {
  // when max_count == 0, the function returns immediately
  {
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto max_waiting_time_point = high_resolution_clock::now() + seconds(10);
    const auto before = high_resolution_clock::now();
    const auto executed = executor->loop_until(0, max_waiting_time_point);
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = duration_cast<milliseconds>(after - before);

    ASSERT_EQ(executed, static_cast<size_t>(0));
    ASSERT_LE(ms_elapsed, milliseconds(5));
  }

  // when deadline <= now, the function returns 0
  {
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto now = high_resolution_clock::now();
    std::this_thread::sleep_for(milliseconds(1));

    ASSERT_EQ(executor->loop_until(100, now), static_cast<size_t>(0));

    executor->post(observer.get_testing_stub());

    ASSERT_EQ(executor->loop_until(100, now), static_cast<size_t>(0));

    ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
    ASSERT_EQ(observer.get_destruction_count(), static_cast<size_t>(0));
  }

  // if max_count is reached, the function returns
  {
    object_observer observer;
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto max_looping_time_point = high_resolution_clock::now() + seconds(10);
    const auto enqueueing_interval = milliseconds(100);
    const size_t max_count = 8;

    std::thread enqueuer([max_count, enqueueing_interval, executor, &observer] {
      for (size_t i = 0; i < max_count + 1; i++) {
        std::this_thread::sleep_for(enqueueing_interval);
        executor->post(observer.get_testing_stub());
      }
    });

    const auto executed = executor->loop_until(max_count, max_looping_time_point);

    ASSERT_EQ(executed, max_count);
    ASSERT_LT(high_resolution_clock::now(), max_looping_time_point);

    enqueuer.join();
  }

  // if shutdown requested, the function returns and throws
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->loop_until(100, high_resolution_clock::now() + seconds(10));
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_clear() {
  object_observer observer;
  const size_t task_count = 100;
  auto executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner shutdown(executor);

  ASSERT_EQ(executor->clear(), static_cast<size_t>(0));

  std::vector<concurrency::Result<size_t>> results;
  results.resize(task_count);

  for (size_t i = 0; i < task_count; i++) {
    results[i] = executor->submit(observer.get_testing_stub(i));
  }

  ASSERT_EQ(executor->clear(), task_count);
  ASSERT_TRUE(executor->empty());
  ASSERT_EQ(executor->size(), static_cast<size_t>(0));
  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));

  for (auto &result : results) {
    assert_throws<std::runtime_error>([&result]() mutable {
      result.get();
    });
  }

  ASSERT_EQ(observer.get_destruction_count(), task_count);

  ASSERT_EQ(executor->clear(), static_cast<size_t>(0));
}

void plain::tests::test_manual_executor_wait_for_task() {
  // case 1: tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    executor->post([] {
    });

    const auto before = high_resolution_clock::now();
    executor->wait_for_task();
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 2: goes to sleep, woken by a task
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    auto enqueuing_time = high_resolution_clock::now() + milliseconds(150);

    std::thread enqueuing_thread([executor, enqueuing_time]() mutable {
      std::this_thread::sleep_until(enqueuing_time);
      executor->post([] {
      });
    });

    ASSERT_EQ(executor->size(), static_cast<size_t>(0));
    executor->wait_for_task();
    const auto now = high_resolution_clock::now();
    ASSERT_EQ(executor->size(), static_cast<size_t>(1));

    ASSERT_GE(now, enqueuing_time);

    enqueuing_thread.join();
  }

  // case 3: goes to sleep, wakes up by an interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->wait_for_task();
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_wait_for_task_for() {
  // case 1: timeout
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    const auto waiting_time_ms = milliseconds(50);
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < 10; i++) {
      const auto before = high_resolution_clock::now();
      ASSERT_FALSE(executor->wait_for_task_for(waiting_time_ms));
      const auto after = high_resolution_clock::now();
      const auto ms_elapsed = std::chrono::duration_cast<milliseconds>(after - before);
      ASSERT_EQ(executor->size(), 0);
      ASSERT_GE(ms_elapsed, waiting_time_ms);
    }
  }

  // case 2: tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    const auto waiting_time_ms = milliseconds(150);
    executor_shutdowner shutdown(executor);

    executor->post([] {
    });

    const auto before = high_resolution_clock::now();
    ASSERT_TRUE(executor->wait_for_task_for(waiting_time_ms));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 3: goes to sleep, then woken by an incoming task
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto enqueuing_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, enqueuing_time]() mutable {
      std::this_thread::sleep_until(enqueuing_time);
      executor->post([] {
      });
    });

    ASSERT_TRUE(executor->wait_for_task_for(seconds(10)));
    const auto now = high_resolution_clock::now();

    ASSERT_EQ(executor->size(), static_cast<size_t>(1));
    ASSERT_GE(now, enqueuing_time);
    ASSERT_LE(now, enqueuing_time + seconds(1));

    thread.join();
  }

  // case 4: goes to sleep, then woken by an interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->wait_for_task_for(seconds(10));
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_wait_for_task_until() {
  // case 1: timeout
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < 10; i++) {
      const auto max_waiting_time_point =
        high_resolution_clock::now() + milliseconds(50);
      ASSERT_FALSE(executor->wait_for_task_until(max_waiting_time_point));
      const auto now = high_resolution_clock::now();
      ASSERT_GE(now, max_waiting_time_point);
    }
  }

  // case 2: tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto max_waiting_time_point =
      high_resolution_clock::now() + milliseconds(150);

    executor->post([] {
    });

    const auto before = high_resolution_clock::now();
    ASSERT_TRUE(executor->wait_for_task_until(max_waiting_time_point));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 3: goes to sleep, then woken by an incoming task
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto enqueue_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, enqueue_time]() mutable {
      std::this_thread::sleep_until(enqueue_time);
      executor->post([] {
      });
    });

    ASSERT_TRUE(executor->wait_for_task_until(
      high_resolution_clock::now() + seconds(10)));
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, enqueue_time);
    ASSERT_LE(now, enqueue_time + seconds(1));

    thread.join();
  }

  // case 4: goes to sleep, then woken by an interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->wait_for_task_until(high_resolution_clock::now() + seconds(10));
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_wait_for_tasks() {
  constexpr size_t task_count = 4;

  // case 0: max_count == 0, the function returns immediately
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto before = high_resolution_clock::now();
    executor->wait_for_tasks(0);
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 1: max_count tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < task_count; i++) {
      executor->post([] {
      });
    }

    const auto before = high_resolution_clock::now();
    executor->wait_for_tasks(4);
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = 
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 2: goes to sleep, woken by incoming tasks
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto enqueuing_interval = milliseconds(100);
    const auto before = high_resolution_clock::now();
    std::thread enqueuing_thread([executor, enqueuing_interval]() mutable {
      for (size_t i = 0; i < task_count; i++) {
        std::this_thread::sleep_for(enqueuing_interval);
        executor->post([] {
        });
      }
    });

    executor->wait_for_tasks(task_count);
    const auto now = high_resolution_clock::now();

    ASSERT_EQ(executor->size(), task_count);
    ASSERT_GE(now, before + enqueuing_interval  *task_count);
    ASSERT_LE(now, before + enqueuing_interval  *task_count + seconds(1));

    enqueuing_thread.join();
  }

  // case 3: goes to sleep, wakes up by an interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->wait_for_tasks(task_count);
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_wait_for_tasks_for() {
  constexpr size_t task_count = 4;

  // case 0: max_count == 0, the function returns immediately
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto before = high_resolution_clock::now();
    executor->wait_for_tasks_for(0, seconds(4));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = 
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 1: timeout
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    const auto waiting_time_ms = milliseconds(50);
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < 10; i++) {
      const auto before = high_resolution_clock::now();
      ASSERT_FALSE(executor->wait_for_tasks_for(task_count, waiting_time_ms));
      const auto after = high_resolution_clock::now();
      const auto ms_elapsed =
        std::chrono::duration_cast<milliseconds>(after - before);
      ASSERT_EQ(executor->size(), 0);
      ASSERT_GE(ms_elapsed, waiting_time_ms);
    }
  }

  // case 2: max_count tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    const auto waiting_time_ms = milliseconds(150);
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < task_count; i++) {
      executor->post([] {
      });
    }

    const auto before = high_resolution_clock::now();
    ASSERT_TRUE(executor->wait_for_tasks_for(task_count, waiting_time_ms));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 3: goes to sleep, then woken by incoming tasks
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto enqueuing_interval = milliseconds(100);
    const auto before = high_resolution_clock::now();

    std::thread enqueuing_thread([executor, enqueuing_interval]() mutable {
      for (size_t i = 0; i < task_count; i++) {
        std::this_thread::sleep_for(enqueuing_interval);
        executor->post([] {
        });
      }
    });

    executor->wait_for_tasks_for(task_count, std::chrono::seconds(10));

    const auto now = high_resolution_clock::now();

    ASSERT_EQ(executor->size(), task_count);
    ASSERT_GE(now, before + enqueuing_interval  *task_count);
    ASSERT_LE(now, before + enqueuing_interval  *task_count + seconds(1));

    enqueuing_thread.join();
  }

  // case 4: goes to sleep, then woken by an interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->wait_for_tasks_for(task_count, seconds(10));
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

void plain::tests::test_manual_executor_wait_for_tasks_until() {
  constexpr size_t task_count = 4;

  // case 0: max_count == 0, the function returns immediately
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    const auto before = high_resolution_clock::now();
    executor->wait_for_tasks_until(0, high_resolution_clock::now() + seconds(4));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed =
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 1: timeout
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);

    for (size_t i = 0; i < 10; i++) {
      const auto max_waiting_time_point =
        high_resolution_clock::now() + milliseconds(50);
      ASSERT_FALSE(
        executor->wait_for_tasks_until(task_count, max_waiting_time_point));
      const auto after = high_resolution_clock::now();
      ASSERT_EQ(executor->size(), 0);
      ASSERT_GE(after, max_waiting_time_point);
    }
  }

  // case 2: max_count tasks already exist
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto max_waiting_time_point =
      high_resolution_clock::now() + milliseconds(150);

    for (size_t i = 0; i < task_count; i++) {
      executor->post([] {
      });
    }

    const auto before = high_resolution_clock::now();
    ASSERT_TRUE(executor->wait_for_tasks_until(
      task_count, max_waiting_time_point));
    const auto after = high_resolution_clock::now();
    const auto ms_elapsed = 
      std::chrono::duration_cast<milliseconds>(after - before).count();
    ASSERT_LE(ms_elapsed, 5);
  }

  // case 3: goes to sleep, then woken by incoming tasks
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto enqueuing_interval = milliseconds(150);

    const auto before = high_resolution_clock::now();

    std::thread enqueuing_thread([executor, enqueuing_interval]() mutable {
      for (size_t i = 0; i < task_count; i++) {
        std::this_thread::sleep_for(enqueuing_interval);
        executor->post([] {
        });
      }
    });

    executor->wait_for_tasks_until(
      task_count, high_resolution_clock::now() + std::chrono::seconds(10));

    const auto now = high_resolution_clock::now();

    ASSERT_EQ(executor->size(), task_count);
    ASSERT_GE(now, before + enqueuing_interval  *task_count);
    ASSERT_LE(now, before + enqueuing_interval  *task_count + seconds(2));

    enqueuing_thread.join();
  }

  // case 4: goes to sleep, then woken by an interrupt
  {
    auto executor = std::make_shared<plain::concurrency::executor::Manual>();
    executor_shutdowner shutdown(executor);
    const auto shutdown_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([executor, shutdown_time]() mutable {
      std::this_thread::sleep_until(shutdown_time);
      executor->shutdown();
    });

    assert_throws<std::runtime_error>([executor] {
      executor->wait_for_tasks_until(
        task_count, high_resolution_clock::now() + seconds(10));
    });

    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, shutdown_time);
    ASSERT_LE(now, shutdown_time + seconds(1));

    thread.join();
  }
}

using namespace plain::tests;
class ManualExecutor : public testing::Test {

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

TEST_F(ManualExecutor, testName) {
  test_manual_executor_name();
}

TEST_F(ManualExecutor, testShutdown) {
  test_manual_executor_shutdown();
}

TEST_F(ManualExecutor, testMaxConcurrencyLevel) {
  test_manual_executor_max_concurrency_level();
}

TEST_F(ManualExecutor, testPost) {
  test_manual_executor_post();
}

TEST_F(ManualExecutor, testSubmit) {
  test_manual_executor_submit();
}

TEST_F(ManualExecutor, testBulkPost) {
  test_manual_executor_bulk_post();
}

TEST_F(ManualExecutor, testBulkSubmit) {
  test_manual_executor_bulk_submit();
}

TEST_F(ManualExecutor, testLoopOnce) {
  test_manual_executor_loop_once();
}

TEST_F(ManualExecutor, testLoopOnceFor) {
  test_manual_executor_loop_once_for();
}

TEST_F(ManualExecutor, testLoopOnceUntil) {
  test_manual_executor_loop_once_until();
}

TEST_F(ManualExecutor, testLoop) {
  test_manual_executor_loop();
}

TEST_F(ManualExecutor, testLoopFor) {
  test_manual_executor_loop_for();
}

TEST_F(ManualExecutor, testLoopUntil) {
  test_manual_executor_loop_until();
}

TEST_F(ManualExecutor, testWaitForTask) {
  test_manual_executor_wait_for_task();
}

TEST_F(ManualExecutor, testWaitForTaskFor) {
  test_manual_executor_wait_for_task_for();
}

TEST_F(ManualExecutor, testWaitForTaskUntil) {
  test_manual_executor_wait_for_task_until();
}

TEST_F(ManualExecutor, testWaitForTasks) {
  test_manual_executor_wait_for_tasks();
}

TEST_F(ManualExecutor, testWaitForTasksFor) {
  test_manual_executor_wait_for_tasks_for();
}

TEST_F(ManualExecutor, testWaitForTasksUntil) {
  test_manual_executor_wait_for_tasks_until();
}

TEST_F(ManualExecutor, testClear) {
  test_manual_executor_clear();
}

/*
int main() {
  tester tester("manual_executor test");

  tester.add_step("name", test_manual_executor_name);
  tester.add_step("shutdown", test_manual_executor_shutdown);
  tester.add_step("max_concurrency_level", test_manual_executor_max_concurrency_level);
  tester.add_step("post", test_manual_executor_post);
  tester.add_step("submit", test_manual_executor_submit);
  tester.add_step("bulk_post", test_manual_executor_bulk_post);
  tester.add_step("bulk_submit", test_manual_executor_bulk_submit);
  tester.add_step("loop_once", test_manual_executor_loop_once);
  tester.add_step("loop_once_for", test_manual_executor_loop_once_for);
  tester.add_step("loop_once_until", test_manual_executor_loop_once_until);
  tester.add_step("loop", test_manual_executor_loop);
  tester.add_step("loop_for", test_manual_executor_loop_for);
  tester.add_step("loop_until", test_manual_executor_loop_until);
  tester.add_step("wait_for_task", test_manual_executor_wait_for_task);
  tester.add_step("wait_for_task_for", test_manual_executor_wait_for_task_for);
  tester.add_step("wait_for_task_until", test_manual_executor_wait_for_task_until);
  tester.add_step("wait_for_tasks", test_manual_executor_wait_for_tasks);
  tester.add_step("wait_for_tasks_for", test_manual_executor_wait_for_tasks_for);
  tester.add_step("wait_for_tasks_until", test_manual_executor_wait_for_tasks_until);
  tester.add_step("clear", test_manual_executor_clear);

  tester.launch_test();
  return 0;
}
*/
