#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"

#include <algorithm>

namespace plain::tests {

template<class type>
void test_when_all_vector_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

template<class type>
void test_when_all_vector_empty_range(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

template<class type>
void test_when_all_vector_null_resume_executor();

template<class type>
concurrency::Result<void> 
test_when_all_vector_valid(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex);

template<class type>
void test_when_all_vector_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

template<class type>
void test_when_all_vector_impl();
void test_when_all_vector();

void test_when_all_tuple_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

void test_when_all_tuple_empty_range(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

void test_when_all_tuple_null_resume_executor();

concurrency::Result<void>
test_when_all_tuple_valid(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex);

void test_when_all_tuple_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

void test_when_all_tuple();

}  // namespace plain::tests

static void local_assert_false(bool value) {
  ASSERT_FALSE(value);
}

static void local_assert_true(bool value) {
  ASSERT_TRUE(value);
}

template <typename T>
static void local_assert_eq(T value1, T value2) {
  ASSERT_EQ(value1, value2);
}

template<class type>
void plain::tests::test_when_all_vector_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  constexpr size_t task_count = 63;
  std::vector<concurrency::ResultPromise<type>> result_promises(task_count);
  std::vector<concurrency::Result<type>> results;

  for (auto &rp : result_promises) {
    results.emplace_back(rp.get_result());
  }

  // place an empty result in the end of the array
  results.emplace_back();

  assert_throws_with_error_message<std::runtime_error>(
    [&results, resume_executor] {
      plain::concurrency::result::when_all(
        resume_executor, results.begin(), results.end());
    },
    "when_all - one of the result objects is empty.");

  const auto all_valid = std::all_of(
    results.begin(), results.begin() + task_count, [](const auto &result) {
    return static_cast<bool>(result);
  });

  ASSERT_TRUE(all_valid);
}

template<class type>
void plain::tests::test_when_all_vector_empty_range(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  std::vector<concurrency::Result<type>> empty_range;
  auto all = plain::concurrency::result::when_all(
    resume_executor, empty_range.begin(), empty_range.end());
  ASSERT_EQ(all.status(), concurrency::ResultStatus::Idle);

  auto all_done = all.run().get();

  ASSERT_TRUE(all_done.empty());
}

template<class type>
void plain::tests::test_when_all_vector_null_resume_executor() {
  constexpr size_t task_count = 8;
  std::vector<concurrency::ResultPromise<type>> result_promises(task_count);
  std::vector<concurrency::Result<type>> results;

  for (auto &rp : result_promises) {
    results.emplace_back(rp.get_result());
  }

  // with results
  assert_throws_with_error_message<std::invalid_argument>(
    [&] {
      concurrency::result::when_all(
        std::shared_ptr<plain::concurrency::executor::WorkerThread> {},
        results.begin(), results.end());
    },
    "when_all - given resume_executor is null.");
}

template<class type>
plain::concurrency::Result<void>
plain::tests::test_when_all_vector_valid(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  constexpr size_t task_count = 1'024;
  std::atomic_size_t counter = 0;
  value_gen<type> gen;
  std::vector<concurrency::Result<type>> results;
  results.reserve(1'024);

  for (size_t i = 0; i < task_count; i++) {
    results.emplace_back(ex->submit([gen, i, &counter]() mutable -> type {
      counter.fetch_add(1, std::memory_order_relaxed);

      if (i % 4 == 0) {
        throw custom_exception(i);
      }

      if constexpr (!std::is_same_v<void, type>) {
        return gen.value_of(i);
      }
    }));
  }

  auto all = plain::concurrency::result::when_all(
    resume_executor, results.begin(), results.end());

  const auto all_empty = std::all_of(
    results.begin(), results.end(), [](const auto &res) {
    return !static_cast<bool>(res);
  });

  local_assert_true(all_empty);
  local_assert_eq(all.status(), concurrency::ResultStatus::Idle);

  auto done_results = co_await all;

  local_assert_eq(counter.load(std::memory_order_relaxed), results.size());

  const auto all_done = std::all_of(
    done_results.begin(), done_results.end(), [](const auto &res) {
    return static_cast<bool>(res) && 
      (res.status() != concurrency::ResultStatus::Idle);
  });

  local_assert_true(all_done);

  for (size_t i = 0; i < task_count; i++) {
    if (i % 4 == 0) {
      test_ready_result_custom_exception(std::move(done_results[i]), i);
    } else {
      if constexpr (!std::is_same_v<void, type>) {
        test_ready_result(std::move(done_results[i]), gen.value_of(i));
      } else {
        test_ready_result(std::move(done_results[i]));
      }
    }
  }
}

namespace plain::tests {

template<class type>
concurrency::Result<void> 
await_result_when_all(
  std::atomic_uintptr_t &resuming_thread_id, concurrency::Result<type> res) {
  co_await res;
  resuming_thread_id.store(plain::thread::get_current_virtual_id());
}

}  // namespace plain::tests

template<class type>
void plain::tests::test_when_all_vector_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  constexpr size_t task_count = 16;
  std::vector<concurrency::ResultPromise<type>> result_promises(task_count);
  std::vector<concurrency::Result<type>> results;

  for (auto &rp : result_promises) {
    results.emplace_back(rp.get_result());
  }

  std::atomic_uintptr_t this_thread_id = 
    plain::thread::get_current_virtual_id(), resuming_thread_id {0};

  auto all = concurrency::result::when_all(
    resume_executor, results.begin(), results.end()).run();

  auto test = await_result_when_all(resuming_thread_id, std::move(all));

  for (auto &rp : result_promises) {
    rp.set_from_function(value_gen<type>::default_value);
  }

  test.get();

  // the coroutine was resumed using worker_thread_executor, hence the thread id should be different.
  ASSERT_NE(this_thread_id.load(), resuming_thread_id.load());
}

template<class type>
void plain::tests::test_when_all_vector_impl() {
  const auto wte = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown0(wte);

  test_when_all_vector_empty_result<type>(wte);
  test_when_all_vector_empty_range<type>(wte);
  test_when_all_vector_null_resume_executor<type>();

  const auto ex = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner shutdown1(ex);

  test_when_all_vector_valid<type>(wte, ex).get();
  test_when_all_vector_resuming_mechanism<type>(wte);
}

void plain::tests::test_when_all_vector() {
  test_when_all_vector_impl<int>();
  test_when_all_vector_impl<std::string>();
  test_when_all_vector_impl<void>();
  test_when_all_vector_impl<int&>();
  test_when_all_vector_impl<std::string&>();
}

void plain::tests::test_when_all_tuple_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  concurrency::ResultPromise<int> rp_int;
  auto int_res = rp_int.get_result();

  concurrency::ResultPromise<std::string> rp_str;
  auto str_res = rp_str.get_result();

  concurrency::ResultPromise<void> rp_void;
  auto void_res = rp_void.get_result();

  concurrency::ResultPromise<int&> rp_int_ref;
  auto int_ref_res = rp_int_ref.get_result();

  concurrency::Result<std::string&> s_ref_res;

  assert_throws_with_error_message<std::runtime_error>(
    [&] {
      concurrency::result::when_all(resume_executor,
           std::move(int_res),
           std::move(str_res),
           std::move(void_res),
           std::move(int_ref_res),
           std::move(s_ref_res));
    },
    "when_all - one of the result objects is empty.");

  ASSERT_TRUE(static_cast<bool>(int_res));
  ASSERT_TRUE(static_cast<bool>(str_res));
  ASSERT_TRUE(static_cast<bool>(void_res));
  ASSERT_TRUE(static_cast<bool>(int_res));
}

void plain::tests::test_when_all_tuple_null_resume_executor() {
  concurrency::ResultPromise<int> rp_int;
  auto int_res = rp_int.get_result();

  concurrency::ResultPromise<std::string> rp_str;
  auto str_res = rp_str.get_result();

  concurrency::ResultPromise<void> rp_void;
  auto void_res = rp_void.get_result();

  // with results
  assert_throws_with_error_message<std::invalid_argument>(
    [&] {
      concurrency::result::when_all(
        std::shared_ptr<plain::concurrency::executor::WorkerThread> {},
        std::move(int_res), std::move(str_res), std::move(void_res));
    },
    "when_all - given resume_executor is null.");

  // no results
  assert_throws_with_error_message<std::invalid_argument>(
    [] {
      concurrency::result::when_all(
        std::shared_ptr<plain::concurrency::executor::WorkerThread> {});
    },
    "when_all - given resume_executor is null.");
}

void plain::tests::test_when_all_tuple_empty_range(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  auto all = concurrency::result::when_all(resume_executor);
  ASSERT_EQ(all.status(), concurrency::ResultStatus::Idle);

  auto eager_all = all.run();

  ASSERT_EQ(eager_all.status(), concurrency::ResultStatus::Value);
  ASSERT_EQ(eager_all.get(), std::tuple<> {});
}

plain::concurrency::Result<void> 
plain::tests::test_when_all_tuple_valid(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  std::atomic_size_t counter = 0;

  auto int_res_val = ex->submit([&] -> int {
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<int>::default_value();
  });

  auto int_res_ex = ex->submit([&] -> int {
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(0);
    return value_gen<int>::default_value();
  });

  auto s_res_val = ex->submit([&] -> std::string {
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<std::string>::default_value();
  });

  auto s_res_ex = ex->submit([&] -> std::string {
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(1);
    return value_gen<std::string>::default_value();
  });

  auto void_res_val = ex->submit([&] {
    counter.fetch_add(1, std::memory_order_relaxed);
  });

  auto void_res_ex = ex->submit([&] {
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(2);
  });

  auto int_ref_res_val = ex->submit([&] -> int &{
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<int&>::default_value();
  });

  auto int_ref_res_ex = ex->submit([&] -> int &{
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(3);
    return value_gen<int&>::default_value();
  });

  auto str_ref_res_val = ex->submit([&] -> std::string &{
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<std::string&>::default_value();
  });

  auto str_ref_res_ex = ex->submit([&] -> std::string &{
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(4);
    return value_gen<std::string&>::default_value();
  });

  auto all = concurrency::result::when_all(resume_executor,
            std::move(int_res_val),
            std::move(int_res_ex),
            std::move(s_res_val),
            std::move(s_res_ex),
            std::move(void_res_val),
            std::move(void_res_ex),
            std::move(int_ref_res_val),
            std::move(int_ref_res_ex),
            std::move(str_ref_res_val),
            std::move(str_ref_res_ex));

  local_assert_false(static_cast<bool>(int_res_val));
  local_assert_false(static_cast<bool>(int_res_ex));
  local_assert_false(static_cast<bool>(s_res_val));
  local_assert_false(static_cast<bool>(s_res_ex));
  local_assert_false(static_cast<bool>(void_res_val));
  local_assert_false(static_cast<bool>(void_res_ex));
  local_assert_false(static_cast<bool>(int_ref_res_val));
  local_assert_false(static_cast<bool>(int_ref_res_ex));
  local_assert_false(static_cast<bool>(str_ref_res_val));
  local_assert_false(static_cast<bool>(str_ref_res_ex));

  local_assert_eq(all.status(), concurrency::ResultStatus::Idle);

  auto done_results_tuple = co_await all;

  local_assert_eq(counter.load(std::memory_order_relaxed), static_cast<size_t>(10));

  test_ready_result(std::move(std::get<0>(done_results_tuple)));
  test_ready_result(std::move(std::get<2>(done_results_tuple)));
  test_ready_result(std::move(std::get<4>(done_results_tuple)));
  test_ready_result(std::move(std::get<6>(done_results_tuple)));
  test_ready_result(std::move(std::get<8>(done_results_tuple)));

  test_ready_result_custom_exception(std::move(std::get<1>(done_results_tuple)), 0);
  test_ready_result_custom_exception(std::move(std::get<3>(done_results_tuple)), 1);
  test_ready_result_custom_exception(std::move(std::get<5>(done_results_tuple)), 2);
  test_ready_result_custom_exception(std::move(std::get<7>(done_results_tuple)), 3);
  test_ready_result_custom_exception(std::move(std::get<9>(done_results_tuple)), 4);
}

void plain::tests::test_when_all_tuple_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  concurrency::ResultPromise<int> rp_int;
  auto int_res = rp_int.get_result();

  concurrency::ResultPromise<std::string> rp_str;
  auto str_res = rp_str.get_result();

  concurrency::ResultPromise<void> rp_void;
  auto void_res = rp_void.get_result();

  std::atomic_uintptr_t this_thread_id =
    plain::thread::get_current_virtual_id(), resuming_thread_id {0};

  auto all = concurrency::result::when_all(
    resume_executor, std::move(int_res), std::move(str_res),
    std::move(void_res)).run();
  auto test = await_result_when_all(resuming_thread_id, std::move(all));

  rp_void.set_result();
  rp_str.set_result("");
  rp_int.set_result(0);

  test.get();

  ASSERT_NE(this_thread_id.load(), resuming_thread_id.load());
}

void plain::tests::test_when_all_tuple() {
  const auto wte = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner shutdown0(wte);

  test_when_all_tuple_empty_result(wte);
  test_when_all_tuple_empty_range(wte);
  test_when_all_tuple_null_resume_executor();

  const auto ex = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner shutdown(ex);
  test_when_all_tuple_valid(wte, ex).get();
  test_when_all_tuple_resuming_mechanism(wte);
}

using namespace plain::tests;

class WhenAll : public testing::Test {

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

TEST_F(WhenAll, testBeginAndEnd) {
  test_when_all_vector();
}

TEST_F(WhenAll, testTuple) {
  test_when_all_tuple();
}

/*
int main() {
  tester test("when_all test");

  test.add_step("when_all(begin, end)", test_when_all_vector);
  test.add_step("when_all(result_types &&... results)", test_when_all_tuple);

  test.launch_test();
  return 0;
}
*/
