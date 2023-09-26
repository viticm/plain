#include "plain/all.h"

#include "util/random.h"
#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"

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

template <typename T>
static void local_assert_ge(T value1, T value2) {
  ASSERT_GE(value1, value2);
}

namespace plain::tests {
template<class type>
void test_when_any_vector_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

template<class type>
void test_when_any_vector_empty_range(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor);

template<class type>
void test_when_any_vector_null_resume_executor();

template<class type>
concurrency::Result<void>
test_when_any_vector_valid(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex);

template<class type>
void test_when_any_vector_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> wte);

template<class type>
void test_when_any_vector_impl();

void test_when_any_vector();

void test_when_any_tuple_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> wte);

void test_when_any_tuple_null_resume_executor();

concurrency::Result<void>
test_when_any_tuple_impl(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex);

void test_when_any_tuple_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> wte);

void test_when_any_tuple();

}  // namespace plain::tests

template<class type>
void plain::tests::test_when_any_vector_empty_result(
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
    [&] {
      plain::concurrency::result::when_any(
        resume_executor, results.begin(), results.end());
    },
    "when_any - one of the result objects is empty.");

  const auto all_valid = std::all_of(
    results.begin(), results.begin() + task_count, [](const auto &result) {
    return static_cast<bool>(result);
  });

  ASSERT_TRUE(all_valid);
}

template<class type>
void plain::tests::test_when_any_vector_empty_range(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  std::vector<concurrency::Result<type>> empty_range;

  assert_throws_with_error_message<std::invalid_argument>(
    [&] {
      concurrency::result::when_any(
        resume_executor, empty_range.begin(), empty_range.end());
    },
    "when_any - given range contains no elements.");
}

template<class type>
void plain::tests::test_when_any_vector_null_resume_executor() {
  constexpr size_t task_count = 16;
  std::vector<concurrency::ResultPromise<type>> result_promises(task_count);
  std::vector<concurrency::Result<type>> results;

  for (auto &rp : result_promises) {
    results.emplace_back(rp.get_result());
  }

  // with results
  assert_throws_with_error_message<std::invalid_argument>(
    [&] {
      concurrency::result::when_any(
        std::shared_ptr<plain::concurrency::executor::WorkerThread> {},
        results.begin(), results.end());
    },
    "when_any - given resume_executor is null.");
}

template<class type>
plain::concurrency::Result<void>
plain::tests::test_when_any_vector_valid(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  constexpr size_t task_count = 64;
  value_gen<type> gen;
  std::vector<concurrency::Result<type>> results;
  random randomizer;

  for (size_t i = 0; i < task_count; i++) {
    const auto time_to_sleep = randomizer(10, 100);
    results.emplace_back(ex->submit([gen, i, time_to_sleep]() mutable -> type {
      std::this_thread::sleep_for(std::chrono::milliseconds(time_to_sleep));

      if (i % 4 == 0) {
        throw custom_exception(i);
      }

      return gen.value_of(i);
    }));
  }

  auto any_res = concurrency::result::when_any(
    resume_executor, results.begin(), results.end());

  const auto all_empty = std::all_of(
    results.begin(), results.end(), [](const auto &result) {
    return !static_cast<bool>(result);
  });

  local_assert_true(all_empty);
  local_assert_eq(any_res.status(), concurrency::ResultStatus::Idle);

  auto any_done = co_await any_res;

  const auto all_valid = std::all_of(
    any_done.results.begin(), any_done.results.end(), [](const auto &result) {
    return static_cast<bool>(result);
  });

  local_assert_true(all_valid);

  auto &done_result = any_done.results[any_done.index];

  if (any_done.index % 4 == 0) {
    test_ready_result_custom_exception(std::move(done_result), any_done.index);
  } else {
    if constexpr (std::is_same_v<void, type>) {
      test_ready_result(std::move(done_result));
    } else {
      test_ready_result(std::move(done_result), gen.value_of(any_done.index));
    }
  }

  // the value vector is a local variable, tasks may outlive it. join them.
  for (auto &result : any_done.results) {
    if (!static_cast<bool>(result)) {
      continue;
    }

    co_await result.resolve();
  }
}

namespace plain::tests {
  
template<class type>
concurrency::Result<void> 
await_result_when_any(
  std::atomic_uintptr_t &resuming_thread_id, concurrency::Result<type> res) {
  co_await res;
  resuming_thread_id.store(plain::thread::get_current_virtual_id());
}

}  // namespace plain::tests

template<class type>
void plain::tests::test_when_any_vector_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  constexpr size_t task_count = 16;
  std::vector<concurrency::ResultPromise<type>> result_promises(task_count);
  std::vector<concurrency::Result<type>> results;

  for (auto &rp : result_promises) {
    results.emplace_back(rp.get_result());
  }

  std::atomic_uintptr_t this_thread_id = 
    plain::thread::get_current_virtual_id(), resuming_thread_id {0};

  auto any = concurrency::result::when_any(
    resume_executor, results.begin(), results.end()).run();

  auto test = await_result_when_any(resuming_thread_id, std::move(any));

  result_promises[10].set_from_function(value_gen<type>::default_value);

  test.get();

  ASSERT_NE(this_thread_id.load(), resuming_thread_id.load());
}

template<class type>
void plain::tests::test_when_any_vector_impl() {
  auto wte = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner es1(wte);

  test_when_any_vector_empty_result<type>(wte);
  test_when_any_vector_empty_range<type>(wte);
  test_when_any_vector_null_resume_executor<type>();

  auto ex = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner es0(ex);

  test_when_any_vector_valid<type>(wte, ex).get();
  test_when_any_vector_resuming_mechanism<type>(wte);
}

void plain::tests::test_when_any_vector() {
  test_when_any_vector_impl<int>();
  test_when_any_vector_impl<std::string>();
  test_when_any_vector_impl<void>();
  test_when_any_vector_impl<int&>();
  test_when_any_vector_impl<std::string&>();
}

void plain::tests::test_when_any_tuple_empty_result(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  concurrency::ResultPromise<int> rp_int;
  auto int_res = rp_int.get_result();

  concurrency::ResultPromise<std::string> rp_str;
  auto str_res = rp_str.get_result();

  concurrency::ResultPromise<void> rp_void;
  auto void_res = rp_void.get_result();

  concurrency::ResultPromise<int&> rp_int_ref;
  auto int_ref_res = rp_int_ref.get_result();

  concurrency::Result<std::string&> str_ref_res;

  assert_throws_with_error_message<std::runtime_error>(
    [&] {
      concurrency::result::when_any(resume_executor,
           std::move(int_res),
           std::move(str_res),
           std::move(void_res),
           std::move(int_ref_res),
           std::move(str_ref_res));
    },
    "when_any - the function must accept at least one result object.");

  // all pre-operation results are still valid
  ASSERT_TRUE(static_cast<bool>(int_res));
  ASSERT_TRUE(static_cast<bool>(str_res));
  ASSERT_TRUE(static_cast<bool>(void_res));
  ASSERT_TRUE(static_cast<bool>(int_res));
}

void plain::tests::test_when_any_tuple_null_resume_executor() {
  concurrency::ResultPromise<int> rp_int;
  auto int_res = rp_int.get_result();

  concurrency::ResultPromise<std::string> rp_str;
  auto str_res = rp_str.get_result();

  concurrency::ResultPromise<void> rp_void;
  auto void_res = rp_void.get_result();

  // with results
  assert_throws_with_error_message<std::invalid_argument>(
    [&] {
      concurrency::result::when_any(
        std::shared_ptr<plain::concurrency::executor::WorkerThread> {},
        std::move(int_res), std::move(str_res), std::move(void_res));
    },
    "when_any - given resume_executor is null.");
}

plain::concurrency::Result<void>
plain::tests::test_when_any_tuple_impl(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor,
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  std::atomic_size_t counter = 0;
  random randomizer;

  auto tts = randomizer(10, 100);
  auto int_res_val = ex->submit([&counter, tts]() -> int {
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<int>::default_value();
  });

  tts = randomizer(10, 100);
  auto int_res_ex = ex->submit([&counter, tts]() -> int {
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(0);
    return value_gen<int>::default_value();
  });

  tts = randomizer(10, 100);
  auto s_res_val = ex->submit([&counter, tts]() -> std::string {
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<std::string>::default_value();
  });

  tts = randomizer(10, 100);
  auto s_res_ex = ex->submit([&counter, tts]() -> std::string {
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(1);
    return value_gen<std::string>::default_value();
  });

  tts = randomizer(10, 100);
  auto void_res_val = ex->submit([&counter, tts] {
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
  });

  tts = randomizer(10, 100);
  auto void_res_ex = ex->submit([&counter, tts] {
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(2);
  });

  tts = randomizer(10, 100);
  auto int_ref_res_val = ex->submit([&counter, tts]() -> int &{
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<int&>::default_value();
  });

  tts = randomizer(10, 100);
  auto int_ref_res_ex = ex->submit([&counter, tts]() -> int &{
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(3);
    return value_gen<int&>::default_value();
  });

  tts = randomizer(10, 100);
  auto str_ref_res_val = ex->submit([&counter, tts]() -> std::string &{
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    return value_gen<std::string&>::default_value();
  });

  tts = randomizer(10, 100);
  auto str_ref_res_ex = ex->submit([&counter, tts]() -> std::string &{
    std::this_thread::sleep_for(std::chrono::milliseconds(tts));
    counter.fetch_add(1, std::memory_order_relaxed);
    throw custom_exception(4);
    return value_gen<std::string&>::default_value();
  });

  auto any_res = concurrency::result::when_any(resume_executor,
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

  local_assert_eq(any_res.status(), concurrency::ResultStatus::Idle);

  auto any_done = co_await any_res;

  local_assert_ge(counter.load(std::memory_order_relaxed), static_cast<size_t>(1));

  switch (any_done.index) {
    case 0: {
      test_ready_result(std::move(std::get<0>(any_done.results)));
      break;
    }
    case 1: {
      test_ready_result_custom_exception(
        std::move(std::get<1>(any_done.results)), 0);
      break;
    }
    case 2: {
      test_ready_result(
        std::move(std::get<2>(any_done.results)));
      break;
    }
    case 3: {
      test_ready_result_custom_exception(
        std::move(std::get<3>(any_done.results)), 1);
      break;
    }
    case 4: {
      test_ready_result(std::move(std::get<4>(any_done.results)));
      break;
    }
    case 5: {
      test_ready_result_custom_exception(
        std::move(std::get<5>(any_done.results)), 2);
      break;
    }
    case 6: {
      test_ready_result(std::move(std::get<6>(any_done.results)));
      break;
    }
    case 7: {
      test_ready_result_custom_exception(
        std::move(std::get<7>(any_done.results)), 3);
      break;
    }
    case 8: {
      test_ready_result(std::move(std::get<8>(any_done.results)));
      break;
    }
    case 9: {
      test_ready_result_custom_exception(
        std::move(std::get<9>(any_done.results)), 4);
      break;
    }
    default: {
      local_assert_false(true);
    }
  }

  auto wait = [](auto &result) {
    if (static_cast<bool>(result)) {
      result.wait();
    }
  };

  std::apply(
    [wait](auto&... results) {
      (wait(results), ...);
    },
    any_done.results);
}

void plain::tests::test_when_any_tuple_resuming_mechanism(
  std::shared_ptr<plain::concurrency::executor::WorkerThread> resume_executor) {
  concurrency::ResultPromise<int> rp_int;
  auto int_res = rp_int.get_result();

  concurrency::ResultPromise<std::string> rp_str;
  auto str_res = rp_str.get_result();

  concurrency::ResultPromise<void> rp_void;
  auto void_res = rp_void.get_result();

  std::atomic_uintptr_t this_thread_id = 
    plain::thread::get_current_virtual_id(), resuming_thread_id {0};

  auto any = concurrency::result::when_any(
    resume_executor, std::move(int_res), std::move(str_res),
    std::move(void_res)).run();
  auto test = await_result_when_any(resuming_thread_id, std::move(any));

  rp_str.set_result("");

  test.get();

  ASSERT_NE(this_thread_id.load(), resuming_thread_id.load());
}

void plain::tests::test_when_any_tuple() {
  auto wte = std::make_shared<plain::concurrency::executor::WorkerThread>();
  executor_shutdowner es0(wte);

  test_when_any_tuple_empty_result(wte);
  test_when_any_tuple_null_resume_executor();

  auto ex = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner es1(ex);
  test_when_any_tuple_impl(wte, ex).get();
  test_when_any_tuple_resuming_mechanism(wte);
}

using namespace plain::tests;

class WhenAny : public testing::Test {

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

TEST_F(WhenAny, testBeginAndEnd) {
  test_when_any_vector();
}

TEST_F(WhenAny, testTuple) {
  test_when_any_tuple();
}

/*
int main() {
  tester test("when_any test");

  test.add_step("when_any(begin, end)", test_when_any_vector);
  test.add_step("when_any(result_types &&... results)", test_when_any_tuple);

  test.launch_test();
  return 0;
}
*/
