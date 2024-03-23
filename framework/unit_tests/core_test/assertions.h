#ifndef PLAIN_CORE_TEST_ASSERTIONS_H_
#define PLAIN_CORE_TEST_ASSERTIONS_H_

#include <cstdint>
#include <string>
#include <string_view>
#include "gtest/gtest.h"

namespace plain::tests {

template<class exception_type, class task_type>
void assert_throws(task_type &&task) {
  try {
    task();
  } catch (const exception_type&) {
    return;
  } catch (...) {

  }

  ASSERT_FALSE(true);
}

template<class exception_type, class task_type>
void assert_throws_with_error_message(
  task_type &&task, std::string_view error_msg) {
  try {
    task();
  } catch (const exception_type &e) {
    std::string_view _error_msg = e.what();
    ASSERT_EQ(error_msg, _error_msg);
    return;
  } catch (...) {
  }

  ASSERT_FALSE(true);
}

template<class exception_type, class task_type>
void assert_throws_contains_error_message(
  task_type &&task, std::string_view error_msg) {
  try {
    task();
  } catch (const exception_type &e) {
    const auto pos = std::string(e.what()).find(error_msg);
    ASSERT_NE(pos, std::string::npos);
    return;
  } catch (...) {
  }

  ASSERT_FALSE(true);
}

}  // namespace plain::tests

#endif  // PLAIN_CORE_TEST_ASSERTIONS_H_
