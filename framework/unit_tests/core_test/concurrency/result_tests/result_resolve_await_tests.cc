#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/executor_shutdowner.h"

namespace plain::tests {

template<class type>
concurrency::Result<void> test_result_resolve_impl_result_ready_value();

template<class type>
concurrency::Result<void> test_result_resolve_impl_result_ready_exception();

template<class type>
concurrency::Result<void>
test_result_resolve_impl_result_not_ready_value(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor);

template<class type>
concurrency::Result<void>
test_result_resolve_impl_result_not_ready_exception(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor);

template<class type>
void test_result_resolve_impl();
void test_result_resolve();

template<class type>
concurrency::Result<void> test_result_await_impl_result_ready_value();

template<class type>
concurrency::Result<void> test_result_await_impl_result_ready_exception();

template<class type>
concurrency::Result<void>
test_result_await_impl_result_not_ready_value(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor);

template<class type>
concurrency::Result<void>
test_result_await_impl_result_not_ready_exception(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor);

template<class type>
void test_result_await_impl();
void test_result_await();

template<class type>
concurrency::Result<type> wrap_co_await(concurrency::Result<type> result) {
  co_return co_await result;
}

}  // namespace plain::tests

using namespace plain;

static void local_assert_false(bool value) {
  ASSERT_FALSE(value);
}

template <typename T>
static void local_assert_eq(T value1, T value2) {
  ASSERT_EQ(value1, value2);
}

/*
  Test result statuses [ready. not ready] vs. result outcome [value, exception]
*/

template<class type>
concurrency::Result<void>
plain::tests::test_result_resolve_impl_result_ready_value() {
  auto result = result_gen<type>::ready();

  const auto thread_id_0 = thread::get_current_virtual_id();

  auto done_result = co_await result.resolve();

  const auto thread_id_1 = thread::get_current_virtual_id();

  local_assert_false(static_cast<bool>(result));
  local_assert_eq(thread_id_0, thread_id_1);
  test_ready_result(std::move(done_result));
}

template<class type>
concurrency::Result<void> plain::tests::test_result_resolve_impl_result_ready_exception() {
  const auto id = 1234567;
  auto result = concurrency::result::make_exceptional<type>(custom_exception(id));

  const auto thread_id_0 = thread::get_current_virtual_id();

  auto done_result = co_await result.resolve();

  const auto thread_id_1 = thread::get_current_virtual_id();

  local_assert_false(static_cast<bool>(result));
  local_assert_eq(thread_id_0, thread_id_1);
  test_ready_result_custom_exception(std::move(done_result), id);
}

template<class type>
concurrency::Result<void>
plain::tests::test_result_resolve_impl_result_not_ready_value(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor) {
  std::atomic_uintptr_t setting_thread_id = 0;

  auto result = thread_executor->submit([&setting_thread_id]() mutable -> type {
    setting_thread_id = thread::get_current_virtual_id();

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    return value_gen<type>::default_value();
  });

  auto done_result = co_await result.resolve();
  test_ready_result(std::move(done_result));
  local_assert_eq(thread::get_current_virtual_id(), setting_thread_id.load());
}

template<class type>
concurrency::Result<void>
plain::tests::test_result_resolve_impl_result_not_ready_exception(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor) {
  std::atomic_uintptr_t setting_thread_id = 0;

  auto result = thread_executor->submit([&setting_thread_id]() mutable -> type {
    setting_thread_id = thread::get_current_virtual_id();

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    return value_gen<type>::throw_ex();
  });

  auto done_result = co_await result.resolve();

  local_assert_eq(done_result.status(), concurrency::ResultStatus::Exception);
  assert_throws<test_exception>([&done_result] {
    done_result.get();
  });

  local_assert_eq(thread::get_current_virtual_id(), setting_thread_id.load());
}

template<class type>
void plain::tests::test_result_resolve_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        concurrency::Result<type>().resolve();
      },
      "resolve - result is empty.");
  }

  auto thread_executor = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner es(thread_executor);

  test_result_resolve_impl_result_ready_value<type>().get();
  test_result_resolve_impl_result_ready_exception<type>().get();
  test_result_resolve_impl_result_not_ready_value<type>(thread_executor).get();
  test_result_resolve_impl_result_not_ready_exception<type>(thread_executor).get();
}

void plain::tests::test_result_resolve() {
  test_result_resolve_impl<int>();
  test_result_resolve_impl<std::string>();
  test_result_resolve_impl<void>();
  test_result_resolve_impl<int&>();
  test_result_resolve_impl<std::string&>();
}

template<class type>
concurrency::Result<void>
plain::tests::test_result_await_impl_result_ready_value() {
  auto result = result_gen<type>::ready();

  const auto thread_id_0 = thread::get_current_virtual_id();

  auto done_result = co_await wrap_co_await(std::move(result)).resolve();

  const auto thread_id_1 = thread::get_current_virtual_id();

  local_assert_false(static_cast<bool>(result));
  local_assert_eq(thread_id_0, thread_id_1);
  test_ready_result(std::move(done_result));
}

template<class type>
concurrency::Result<void>
plain::tests::test_result_await_impl_result_ready_exception() {
  const auto id = 1234567;
  auto result = concurrency::result::make_exceptional<type>(custom_exception(id));

  const auto thread_id_0 = thread::get_current_virtual_id();

  auto done_result = co_await wrap_co_await(std::move(result)).resolve();

  const auto thread_id_1 = thread::get_current_virtual_id();

  local_assert_false(static_cast<bool>(result));
  local_assert_eq(thread_id_0, thread_id_1);
  test_ready_result_custom_exception(std::move(done_result), id);
}

template<class type>
concurrency::Result<void>
plain::tests::test_result_await_impl_result_not_ready_value(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor) {
  std::atomic_uintptr_t setting_thread_id = 0;

  auto result = thread_executor->submit([&setting_thread_id]() mutable -> type {
    setting_thread_id = thread::get_current_virtual_id();

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    return value_gen<type>::default_value();
  });

  auto done_result = co_await wrap_co_await(std::move(result)).resolve();
  test_ready_result(std::move(done_result));
  local_assert_eq(thread::get_current_virtual_id(), setting_thread_id.load());
}

template<class type>
concurrency::Result<void>
plain::tests::test_result_await_impl_result_not_ready_exception(
  std::shared_ptr<plain::concurrency::executor::Thread> thread_executor) {
  std::atomic_uintptr_t setting_thread_id = 0;

  auto result = thread_executor->submit([&setting_thread_id]() mutable -> type {
    setting_thread_id = thread::get_current_virtual_id();

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    return value_gen<type>::throw_ex();
  });

  auto done_result = co_await wrap_co_await(std::move(result)).resolve();

  local_assert_eq(done_result.status(), concurrency::ResultStatus::Exception);
  assert_throws<test_exception>([&done_result] {
    done_result.get();
  });

  local_assert_eq(thread::get_current_virtual_id(), setting_thread_id.load());
}

template<class type>
void plain::tests::test_result_await_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        concurrency::Result<type>().operator co_await();
      },
      "co_await - result is empty.");
  }

  auto thread_executor = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner es(thread_executor);

  test_result_await_impl_result_ready_value<type>().get();
  test_result_await_impl_result_ready_exception<type>().get();
  test_result_await_impl_result_not_ready_value<type>(thread_executor).get();
  test_result_await_impl_result_not_ready_exception<type>(thread_executor).get();
}

void plain::tests::test_result_await() {
  test_result_await_impl<int>();
  test_result_await_impl<std::string>();
  test_result_await_impl<void>();
  test_result_await_impl<int&>();
  test_result_await_impl<std::string&>();
}

using namespace plain::tests;

class ResultResolveAwait : public testing::Test {

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

TEST_F(ResultResolveAwait, testResolve) {
  test_result_resolve();
}

TEST_F(ResultResolveAwait, testCoAwait) {
  test_result_await();
}

/*
int main() {
  tester tester("result::resolve + result::await");

  tester.add_step("resolve", test_result_resolve);
  tester.add_step("co_await", test_result_await);

  tester.launch_test();
  return 0;
}
*/
