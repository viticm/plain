#ifndef PLAIN_CORE_TEST_TEST_READY_LAZY_RESULT_H_
#define PLAIN_CORE_TEST_TEST_READY_LAZY_RESULT_H_

#include "plain/all.h"

#include "assertions.h"
#include "util/test_generators.h"
#include "util/custom_exception.h"

#include <algorithm>

namespace plain::tests {

void lazy_assert_true(bool value) {
  ASSERT_TRUE(value);
}

template <typename T>
void lazy_assert_eq(T value1, T value2) {
  ASSERT_EQ(value1, value2);
}

template<class type>
concurrency::result::null 
test_ready_lazy_result(
  ::plain::concurrency::LazyResult<type> result, const type &o) {
  lazy_assert_true(static_cast<bool>(result));
  lazy_assert_eq(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    lazy_assert_eq(co_await result, o);
  } catch (...) {
    lazy_assert_true(false);
  }
}

template<class type>
concurrency::result::null 
test_ready_lazy_result(
  ::plain::concurrency::LazyResult<type> result,
  std::reference_wrapper<typename std::remove_reference_t<type>> ref) {
  lazy_assert_true(static_cast<bool>(result));
  lazy_assert_eq(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    auto &result_ref = co_await result;
    lazy_assert_eq(&result_ref, &ref.get());
  } catch (...) {
    lazy_assert_true(false);
  }
}

template<class type>
concurrency::result::null
test_ready_lazy_result(::plain::concurrency::LazyResult<type> result) {
  return test_ready_lazy_result<type>(
    std::move(result), value_gen<type>::default_value());
}

template<>
inline concurrency::result::null 
test_ready_lazy_result(::plain::concurrency::LazyResult<void> result) {
  lazy_assert_true(static_cast<bool>(result));
  lazy_assert_eq(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    co_await result;  // just make sure no exception is thrown.
  } catch (...) {
    lazy_assert_true(false);
  }
}

template<class type>
concurrency::result::null 
test_ready_lazy_result_custom_exception(
  plain::concurrency::LazyResult<type> result, const intptr_t id) {
  lazy_assert_true(static_cast<bool>(result));
  lazy_assert_eq(result.status(), plain::concurrency::ResultStatus::Exception);

  try {
    co_await result;
  } catch (const custom_exception &e) {
    lazy_assert_eq(e.id, id);
    co_return;
  } catch (...) {
  }

  lazy_assert_true(false);
}
}  // namespace plain::tests

#endif
