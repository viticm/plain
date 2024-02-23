#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"
#include "util/throwing_executor.h"
#include "util/executor_shutdowner.h"

namespace plain::tests {

template<class type>
void test_shared_result_resolve_impl();
void test_shared_result_resolve();

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

/*
  * In this test suit, we need to check all the possible scenarios that
  * result::resolve can have.
  * Our tests are split into 2 branches: when the result is already ready at
  * the moment of resolving and
  * when it's not.
 *
  * If the result is already ready, the test matrix looks like this:
  * status[value, exception]
  * Overall 2 scenarios
 *
  *If the result is not ready, the test matrix looks like this:
  *status[value, exception]
  *Overall 2 scenarios
 */

namespace plain::tests {
  template<class type, plain::concurrency::ResultStatus status>
  struct test_await_ready_result {
    plain::concurrency::Result<void> operator()();
  };

  template<class type>
  struct test_await_ready_result<type, plain::concurrency::ResultStatus::Value> {
    plain::concurrency::Result<void> operator()() {
      auto result = result_gen<type>::ready();
      plain::concurrency::result::Shared<type> sr(std::move(result));

      const auto thread_id_0 = thread::get_current_virtual_id();

      auto done_result = co_await sr.resolve();

      const auto thread_id_1 = thread::get_current_virtual_id();

      local_assert_false(static_cast<bool>(result));
      local_assert_eq(thread_id_0, thread_id_1);
      test_ready_result(std::move(done_result));
    }
  };

  template<class type>
  struct test_await_ready_result<type, plain::concurrency::ResultStatus::Exception> {
    plain::concurrency::Result<void> operator()() {
      const auto id = 1234567;
      auto result = 
        plain::concurrency::result::make_exceptional<type>(custom_exception(id));
      plain::concurrency::result::Shared<type> sr(std::move(result));

      const auto thread_id_0 = plain::thread::get_current_virtual_id();

      auto done_result = co_await sr.resolve();

      const auto thread_id_1 = plain::thread::get_current_virtual_id();

      local_assert_false(static_cast<bool>(result));
      local_assert_eq(thread_id_0, thread_id_1);
      test_ready_result_custom_exception(std::move(done_result), id);
    }
  };

  template<class type, plain::concurrency::ResultStatus status>
  struct test_await_not_ready_result {
    plain::concurrency::Result<void>
    operator()(std::shared_ptr<plain::concurrency::executor::Thread> executor);
  };

  template<class type>
  struct test_await_not_ready_result<type, plain::concurrency::ResultStatus::Value> {

   private:
    uintptr_t setting_thread_id_ = 0;
    uintptr_t resuming_thread_id_ = 0;

    plain::concurrency::Result<void> 
    inner_task(
      std::shared_ptr<plain::concurrency::executor::Manual> manual_executor) {
      auto result = manual_executor->submit([]() -> decltype(auto) {
        return value_gen<type>::default_value();
      });
      plain::concurrency::result::Shared<type> sr(std::move(result));

      auto done_result = co_await sr.resolve();

      resuming_thread_id_ = thread::get_current_virtual_id();

      test_ready_result(std::move(done_result));
    }

   public:
    plain::concurrency::Result<void>
    operator()(
      std::shared_ptr<plain::concurrency::executor::Manual> manual_executor,
      std::shared_ptr<plain::concurrency::executor::Thread> thread_executor) {
      local_assert_true(manual_executor->empty());

      auto result = inner_task(manual_executor);

      co_await thread_executor->submit([this, manual_executor] {
        setting_thread_id_ = thread::get_current_virtual_id();
        ASSERT_TRUE(manual_executor->loop_once());
      });

      co_await result;

      local_assert_eq(setting_thread_id_, resuming_thread_id_);
    }
  };

  template<class type>
  struct test_await_not_ready_result<type, plain::concurrency::ResultStatus::Exception> {

   private:
    uintptr_t setting_thread_id_ = 0;
    uintptr_t resuming_thread_id_ = 0;

    plain::concurrency::Result<void> 
    inner_task(
      std::shared_ptr<plain::concurrency::executor::Manual> manual_executor) {
      const auto id = 1234567;
      auto result = manual_executor->submit([id]() -> decltype(auto) {
        throw custom_exception(id);
        return value_gen<type>::default_value();
      });
      plain::concurrency::result::Shared<type> sr(std::move(result));

      auto done_result = co_await sr.resolve();

      resuming_thread_id_ = thread::get_current_virtual_id();

      test_ready_result_custom_exception(std::move(done_result), id);
    }

   public:
    plain::concurrency::Result<void>
    operator()(
      std::shared_ptr<plain::concurrency::executor::Manual> manual_executor,
      std::shared_ptr<plain::concurrency::executor::Thread> thread_executor) {
      local_assert_true(manual_executor->empty());

      auto result = inner_task(manual_executor);

      co_await thread_executor->submit([this, manual_executor] {
        setting_thread_id_ = plain::thread::get_current_virtual_id();
        ASSERT_TRUE(manual_executor->loop_once());
      });

      co_await result;

      local_assert_eq(setting_thread_id_, resuming_thread_id_);
    }
  };

}  // namespace plain::tests

template<class type>
void plain::tests::test_shared_result_resolve_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        plain::concurrency::result::Shared<type>().resolve();
      },
      "resolve - result is empty.");
  }

  // resolve can be called multiple times
  {
    plain::concurrency::result::Shared<type> sr(result_gen<type>::ready());

    for (size_t i = 0; i < 6; i++) {
      sr.resolve();
      ASSERT_TRUE(sr);
    }
  }

  auto thread_executor = std::make_shared<plain::concurrency::executor::Thread>();
  auto manual_executor = std::make_shared<plain::concurrency::executor::Manual>();
  executor_shutdowner es0(thread_executor), es1(manual_executor);

  test_await_ready_result<type, plain::concurrency::ResultStatus::Value>()().get();
  test_await_ready_result<type, plain::concurrency::ResultStatus::Exception>()().get();
  test_await_not_ready_result<type, plain::concurrency::ResultStatus::Value>()(
    manual_executor, thread_executor).get();
  test_await_not_ready_result<type, plain::concurrency::ResultStatus::Exception>()(
    manual_executor, thread_executor).get();
}

void plain::tests::test_shared_result_resolve() {
  test_shared_result_resolve_impl<int>();
  test_shared_result_resolve_impl<std::string>();
  test_shared_result_resolve_impl<void>();
  test_shared_result_resolve_impl<int&>();
  test_shared_result_resolve_impl<std::string&>();
}
using namespace plain::tests;

class SharedResultResolve : public testing::Test {

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

TEST_F(SharedResultResolve, testReslove) {
  test_shared_result_resolve();
}

/*
int main() {
  tester tester("shared_result::resolve");

  tester.add_step("reslove", test_shared_result_resolve);

  tester.launch_test();
  return 0;
}
*/
