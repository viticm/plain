#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/executor_shutdowner.h"
#include "util/test_ready_result.h"
#include "util/test_ready_lazy_result.h"

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
static void local_assert_ne(T value1, T value2) {
  ASSERT_NE(value1, value2);
}

namespace plain::tests {

template<class type>
void test_lazy_result_constructor_impl();
void test_lazy_result_constructor();

template<class type>
void test_lazy_result_destructor_impl();
void test_lazy_result_destructor();

template<class type>
plain::concurrency::Result<void> test_lazy_result_status_impl();
void test_lazy_result_status();

template<class type>
void test_lazy_result_resolve_impl();
void test_lazy_result_resolve();

template<class type>
void test_lazy_result_co_await_operator_impl();
void test_lazy_result_co_await_operator();

template<class type>
void test_lazy_result_run_impl(
  std::shared_ptr<plain::concurrency::executor::Thread> ex);
void test_lazy_result_run();

template<class type>
void test_lazy_result_assignment_operator_self();

template<class type>
void test_lazy_result_assignment_operator_empty_to_empty();

template<class type>
void test_lazy_result_assignment_operator_non_empty_to_empty();

template<class type>
void test_lazy_result_assignment_operator_empty_to_non_empty();

template<class type>
void test_lazy_result_assignment_operator_non_empty_to_non_empty();

template<class type>
void test_lazy_result_assignment_operator_impl();

void test_lazy_result_assignment_operator();

}  // namespace plain::tests

namespace plain::tests {

template<class type>
plain::concurrency::LazyResult<type> sync_lazy_coro() {
  co_return value_gen<type>::default_value();
}

template<class type>
plain::concurrency::LazyResult<type> sync_lazy_coro(testing_stub stub) {
  co_return value_gen<type>::default_value();
}

template<class type>
plain::concurrency::LazyResult<type> sync_lazy_coro(testing_stub stub, value_gen<type> &gen, size_t i) {
  co_return gen.value_of(i);
}

template<class type>
plain::concurrency::LazyResult<type> sync_lazy_coro_ex(intptr_t id) {
  throw custom_exception(id);
  co_return value_gen<type>::default_value();
}

template<class type>
plain::concurrency::LazyResult<type>
async_lazy_coro_val(
  bool &started, std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  started = true;
  co_await plain::concurrency::result::resume_on(ex);
  co_return value_gen<type>::default_value();
}

template<class type>
plain::concurrency::LazyResult<type> 
async_lazy_coro_ex(
  bool &started, std::shared_ptr<plain::concurrency::executor::Thread> ex,
  intptr_t id) {
  started = true;
  co_await plain::concurrency::result::resume_on(ex);
  throw custom_exception(id);
  co_return value_gen<type>::default_value();
}

}  // namespace plain::tests

template<class type>
void plain::tests::test_lazy_result_constructor_impl() {
  // default ctor
  {
    plain::concurrency::LazyResult<type> result;
    ASSERT_FALSE(static_cast<bool>(result));
  }

  // move ctor
  {
    plain::concurrency::LazyResult<type> result = sync_lazy_coro<type>();
    ASSERT_TRUE(static_cast<bool>(result));

    auto other = std::move(result);
    ASSERT_TRUE(static_cast<bool>(other));
    ASSERT_FALSE(static_cast<bool>(result));
  }
}

void plain::tests::test_lazy_result_constructor() {
  test_lazy_result_constructor_impl<int>();
  test_lazy_result_constructor_impl<std::string>();
  test_lazy_result_constructor_impl<void>();
  test_lazy_result_constructor_impl<int&>();
  test_lazy_result_constructor_impl<std::string&>();
}

template<class type>
void plain::tests::test_lazy_result_destructor_impl() {
  object_observer observer;

  {
    auto result = sync_lazy_coro<type>(observer.get_testing_stub());
    ASSERT_EQ(observer.get_destruction_count(), 0);
  }

  ASSERT_EQ(observer.get_destruction_count(), 1);
}

void plain::tests::test_lazy_result_destructor() {
  test_lazy_result_destructor_impl<int>();
  test_lazy_result_destructor_impl<std::string>();
  test_lazy_result_destructor_impl<void>();
  test_lazy_result_destructor_impl<int&>();
  test_lazy_result_destructor_impl<std::string&>();
}

template<class type>
plain::concurrency::Result<void> plain::tests::test_lazy_result_status_impl() {
  assert_throws_with_error_message<std::runtime_error>(
    [] {
      plain::concurrency::LazyResult<type>().status();
    },
    "status - result is empty.");

  // value
  {
    plain::concurrency::LazyResult<type> result = sync_lazy_coro<type>();
    local_assert_eq(result.status(), plain::concurrency::ResultStatus::Idle);

    const auto done_result = co_await result.resolve();
    local_assert_eq(done_result.status(), plain::concurrency::ResultStatus::Value);
  }

  // exception
  {
    plain::concurrency::LazyResult<type> result = sync_lazy_coro_ex<type>(12345);
    local_assert_eq(result.status(), plain::concurrency::ResultStatus::Idle);

    const auto done_result = co_await result.resolve();
    local_assert_eq(done_result.status(), plain::concurrency::ResultStatus::Exception);
  }
}

void plain::tests::test_lazy_result_status() {
  test_lazy_result_status_impl<int>().get();
  test_lazy_result_status_impl<std::string>().get();
  test_lazy_result_status_impl<void>().get();
  test_lazy_result_status_impl<int&>().get();
  test_lazy_result_status_impl<std::string&>().get();
}

namespace plain::tests {

template<class type>
plain::concurrency::Result<void>
test_lazy_result_resolve_non_ready_coro_val(
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  const auto thread_id_before = thread::get_current_virtual_id();
  auto started = false;
  auto result = async_lazy_coro_val<type>(started, ex);

  local_assert_false(started);
  local_assert_eq(result.status(), plain::concurrency::ResultStatus::Idle);

  auto done_result = co_await result.resolve();
  const auto thread_id_after = thread::get_current_virtual_id();

  test_ready_lazy_result(std::move(done_result));
  local_assert_ne(thread_id_before, thread_id_after);
}

template<class type>
plain::concurrency::Result<void>
test_lazy_result_resolve_non_ready_coro_ex(
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  const auto thread_id_before = thread::get_current_virtual_id();
  constexpr intptr_t id = 987654321;

  auto started = false;
  auto result = async_lazy_coro_ex<type>(started, ex, id);

  local_assert_false(started);
  local_assert_eq(result.status(), plain::concurrency::ResultStatus::Idle);

  auto done_result = co_await result.resolve();
  const auto thread_id_after = thread::get_current_virtual_id();

  test_ready_lazy_result_custom_exception(std::move(done_result), id);
  local_assert_ne(thread_id_before, thread_id_after);
}

template<class type>
plain::concurrency::Result<void> test_lazy_result_resolve_ready_coro_val() {
  auto done_result = co_await sync_lazy_coro<type>().resolve();

  const auto thread_id_before = thread::get_current_virtual_id();

  auto result = co_await done_result.resolve();

  const auto thread_id_after = thread::get_current_virtual_id();

  test_ready_lazy_result(std::move(result));
  local_assert_eq(thread_id_before, thread_id_after);
}

template<class type>
plain::concurrency::Result<void> test_lazy_result_resolve_ready_coro_ex() {
  constexpr intptr_t id = 987654321;
  auto done_result = co_await sync_lazy_coro_ex<type>(id).resolve();

  const auto thread_id_before = thread::get_current_virtual_id();

  auto result = co_await done_result.resolve();

  const auto thread_id_after = thread::get_current_virtual_id();

  test_ready_lazy_result_custom_exception(std::move(result), id);
  local_assert_eq(thread_id_before, thread_id_after);
}

}  // namespace plain::tests

template<class type>
void plain::tests::test_lazy_result_resolve_impl() {
  assert_throws_with_error_message<std::runtime_error>(
    [] {
      plain::concurrency::LazyResult<type>().resolve();
    },
    "resolve - result is empty.");

  auto ex = std::make_shared<plain::concurrency::executor::Thread>();
  executor_shutdowner es(ex);

  test_lazy_result_resolve_non_ready_coro_val<type>(ex).get();
  test_lazy_result_resolve_non_ready_coro_ex<type>(ex).get();
  test_lazy_result_resolve_ready_coro_val<type>().get();
  test_lazy_result_resolve_ready_coro_ex<type>().get();
}

void plain::tests::test_lazy_result_resolve() {
  test_lazy_result_resolve_impl<int>();
  test_lazy_result_resolve_impl<std::string>();
  test_lazy_result_resolve_impl<void>();
  test_lazy_result_resolve_impl<int&>();
  test_lazy_result_resolve_impl<std::string&>();
}

namespace plain::tests {

template<class type>
plain::concurrency::LazyResult<type> proxy_coro(plain::concurrency::LazyResult<type> result) {
  co_return co_await result;
}

template<class type>
plain::concurrency::Result<void> 
test_lazy_result_co_await_non_ready_coro_val(
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  const auto thread_id_before = thread::get_current_virtual_id();
  auto started = false;
  auto result = async_lazy_coro_val<type>(started, ex);
  auto proxy_result = proxy_coro(std::move(result));

  local_assert_false(started);
  local_assert_eq(proxy_result.status(), plain::concurrency::ResultStatus::Idle);

  auto done_result = co_await proxy_result.resolve();
  const auto thread_id_after = thread::get_current_virtual_id();

  local_assert_true(started);
  test_ready_lazy_result(std::move(done_result));
  local_assert_ne(thread_id_before, thread_id_after);
}

template<class type>
plain::concurrency::Result<void> 
test_lazy_result_co_await_non_ready_coro_ex(
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  constexpr intptr_t id = 987654321;
  auto started = false;
  const auto thread_id_before = thread::get_current_virtual_id();

  auto result = async_lazy_coro_ex<type>(started, ex, id);
  auto proxy_result = proxy_coro(std::move(result));

  local_assert_false(started);

  local_assert_eq(proxy_result.status(), plain::concurrency::ResultStatus::Idle);

  auto done_result = co_await proxy_result.resolve();

  const auto thread_id_after = thread::get_current_virtual_id();

  local_assert_true(started);
  test_ready_lazy_result_custom_exception(std::move(done_result), id);
  local_assert_ne(thread_id_before, thread_id_after);
}

template<class type>
plain::concurrency::Result<void> test_lazy_result_co_await_ready_coro_val() {
  auto done_result = co_await proxy_coro(sync_lazy_coro<type>()).resolve();

  const auto thread_id_before = thread::get_current_virtual_id();

  auto result = co_await done_result.resolve();

  const auto thread_id_after = thread::get_current_virtual_id();

  test_ready_lazy_result(std::move(result));
  local_assert_eq(thread_id_before, thread_id_after);
}

template<class type>
plain::concurrency::Result<void> test_lazy_result_co_await_ready_coro_ex() {
  constexpr intptr_t id = 987654321;
  auto done_result = co_await proxy_coro(sync_lazy_coro_ex<type>(id)).resolve();

  const auto thread_id_before = thread::get_current_virtual_id();

  auto result = co_await done_result.resolve();

  const auto thread_id_after = thread::get_current_virtual_id();

  test_ready_lazy_result_custom_exception(std::move(result), 987654321);
  local_assert_eq(thread_id_before, thread_id_after);
}
}  // namespace plain::tests

template<class type>
void plain::tests::test_lazy_result_co_await_operator_impl() {
  assert_throws_with_error_message<std::runtime_error>(
    [] {
      plain::concurrency::LazyResult<type>().operator co_await();
    },
    "co_await - result is empty.");

  Kernel runtime;
  test_lazy_result_co_await_non_ready_coro_val<type>(runtime.thread_executor()).get();
  test_lazy_result_co_await_non_ready_coro_ex<type>(runtime.thread_executor()).get();
  test_lazy_result_co_await_ready_coro_val<type>().get();
  test_lazy_result_co_await_ready_coro_ex<type>().get();
}

void plain::tests::test_lazy_result_co_await_operator() {
  test_lazy_result_co_await_operator_impl<int>();
  test_lazy_result_co_await_operator_impl<std::string>();
  test_lazy_result_co_await_operator_impl<void>();
  test_lazy_result_co_await_operator_impl<int&>();
  test_lazy_result_co_await_operator_impl<std::string&>();
}

template<class type>
void plain::tests::test_lazy_result_run_impl(
  std::shared_ptr<plain::concurrency::executor::Thread> ex) {
  assert_throws_with_error_message<std::runtime_error>(
    [] {
      plain::concurrency::LazyResult<type>().run();
    },
    "run - result is empty.");

  auto started = false;
  auto lazy = async_lazy_coro_val<type>(started, ex);

  local_assert_false(started);

  auto result = lazy.run();

  local_assert_true(started);
  local_assert_false(static_cast<bool>(lazy));
  local_assert_true(static_cast<bool>(result));

  result.wait();

  test_ready_result(std::move(result));
}

void plain::tests::test_lazy_result_run() {
  Kernel runtime;
  test_lazy_result_run_impl<int>(runtime.thread_executor());
  test_lazy_result_run_impl<std::string>(runtime.thread_executor());
  test_lazy_result_run_impl<void>(runtime.thread_executor());
  test_lazy_result_run_impl<int&>(runtime.thread_executor());
  test_lazy_result_run_impl<std::string&>(runtime.thread_executor());
}

template<class type>
void plain::tests::test_lazy_result_assignment_operator_self() {
  object_observer observer;

  {
    auto res = sync_lazy_coro<type>(observer.get_testing_stub());
    local_assert_true(static_cast<bool>(res));
    // FIXME: Move to self
    // res = std::move(res);
    local_assert_true(static_cast<bool>(res));
    res.run().get();
  }

  local_assert_eq(observer.get_destruction_count(), static_cast<size_t>(1));
}

template<class type>
void plain::tests::test_lazy_result_assignment_operator_empty_to_empty() {
  plain::concurrency::LazyResult<type> res0, res1;
  res0 = std::move(res1);

  local_assert_false(static_cast<bool>(res0));
  local_assert_false(static_cast<bool>(res1));
}

template<class type>
void plain::tests::test_lazy_result_assignment_operator_non_empty_to_empty() {
  object_observer observer;
  value_gen<type> gen;

  plain::concurrency::LazyResult<type> res0, res1 = 
    sync_lazy_coro<type>(observer.get_testing_stub(), gen, 1);
  res0 = std::move(res1);

  local_assert_true(static_cast<bool>(res0));
  local_assert_false(static_cast<bool>(res1));

  local_assert_eq(observer.get_destruction_count(), static_cast<size_t>(0));

  if constexpr (!std::is_same_v<void, type>) {
    local_assert_eq(res0.run().get(), gen.value_of(1));
  }
}

template<class type>
void plain::tests::test_lazy_result_assignment_operator_empty_to_non_empty() {
  object_observer observer;

  plain::concurrency::LazyResult<type> res0, res1 =
    sync_lazy_coro<type>(observer.get_testing_stub());
  res1 = std::move(res0);

  local_assert_false(static_cast<bool>(res0));
  local_assert_false(static_cast<bool>(res1));

  local_assert_eq(observer.get_destruction_count(), static_cast<size_t>(1));
}

template<class type>
void plain::tests::test_lazy_result_assignment_operator_non_empty_to_non_empty() {
  object_observer observer;
  value_gen<type> gen;

  plain::concurrency::LazyResult<type> res0 = 
    sync_lazy_coro<type>(observer.get_testing_stub(), gen, 0);
  plain::concurrency::LazyResult<type> res1 = 
    sync_lazy_coro<type>(observer.get_testing_stub(), gen, 1);
  res0 = std::move(res1);

  local_assert_true(static_cast<bool>(res0));
  local_assert_false(static_cast<bool>(res1));

  local_assert_eq(observer.get_destruction_count(), static_cast<size_t>(1));
  if constexpr (!std::is_same_v<void, type>) {
    local_assert_eq(res0.run().get(), gen.value_of(1));
  }
}

template<class type>
void plain::tests::test_lazy_result_assignment_operator_impl() {
  test_lazy_result_assignment_operator_self<type>();
  test_lazy_result_assignment_operator_empty_to_empty<type>();
  test_lazy_result_assignment_operator_non_empty_to_empty<type>();
  test_lazy_result_assignment_operator_empty_to_non_empty<type>();
  test_lazy_result_assignment_operator_non_empty_to_non_empty<type>();
}

void plain::tests::test_lazy_result_assignment_operator() {
  test_lazy_result_assignment_operator_impl<int>();
  test_lazy_result_assignment_operator_impl<std::string>();
  test_lazy_result_assignment_operator_impl<void>();
  test_lazy_result_assignment_operator_impl<int&>();
  test_lazy_result_assignment_operator_impl<std::string&>();
}

using namespace plain::tests;

class LazyResult : public testing::Test {

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

TEST_F(LazyResult, testConstructor) {
  test_lazy_result_constructor();
}

TEST_F(LazyResult, testDestructor) {
  test_lazy_result_destructor();
}

TEST_F(LazyResult, testStatus) {
  test_lazy_result_status();
}

TEST_F(LazyResult, testResolve) {
  test_lazy_result_resolve();
}

TEST_F(LazyResult, testOperatorCoAwait) {
  test_lazy_result_co_await_operator();
}

TEST_F(LazyResult, testRun) {
  test_lazy_result_run();
}

TEST_F(LazyResult, testAssignmentOperator) {
  test_lazy_result_assignment_operator();
}

/*
int main() {
  tester tester("lazy_result test");

  tester.add_step("constructor", test_lazy_result_constructor);
  tester.add_step("destructor", test_lazy_result_destructor);
  tester.add_step("status", test_lazy_result_status);
  tester.add_step("resolve", test_lazy_result_resolve);
  tester.add_step("operator co_await", test_lazy_result_co_await_operator);
  tester.add_step("run", test_lazy_result_run);
  tester.add_step("operator =", test_lazy_result_assignment_operator);

  tester.launch_test();
  return 0;
}
*/
