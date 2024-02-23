#ifndef PLAIN_CORE_TEST_TEST_READY_RESULT_H_
#define PLAIN_CORE_TEST_TEST_READY_RESULT_H_

#include "plain/all.h"

#include "assertions.h"
#include "util/test_generators.h"
#include "util/custom_exception.h"

#include <algorithm>

namespace plain::tests {

template<class type>
void test_same_ref_shared_result(
  ::plain::concurrency::result::Shared<type> &result) noexcept {
  const auto value_ptr = std::addressof(result.get());

  for (size_t i = 0; i < 8; i++) {
    ASSERT_EQ(value_ptr, std::addressof(result.get()));
  }
}

}  // namespace plain::tests

namespace plain::tests {
template<class type>
void test_ready_result(
  ::plain::concurrency::Result<type> result, const type &o) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    ASSERT_EQ(result.get(), o);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

template<class type>
void test_ready_result(
  ::plain::concurrency::result::Shared<type> result, const type &o) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    ASSERT_EQ(result.get(), o);
  } catch (...) {
    ASSERT_TRUE(false);
  }

  test_same_ref_shared_result(result);
}

template<class type>
void test_ready_result(
  ::plain::concurrency::Result<type> result,
  std::reference_wrapper<typename std::remove_reference_t<type>> ref) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    ASSERT_EQ(&result.get(), &ref.get());
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

template<class type>
void test_ready_result(
  ::plain::concurrency::result::Shared<type> result,
  std::reference_wrapper<typename std::remove_reference_t<type>> ref) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    ASSERT_EQ(&result.get(), &ref.get());
  } catch (...) {
    ASSERT_TRUE(false);
  }

  test_same_ref_shared_result(result);
}

template<class type>
void test_ready_result(::plain::concurrency::Result<type> result) {
  test_ready_result<type>(std::move(result), value_gen<type>::default_value());
}

template<>
inline void test_ready_result(::plain::concurrency::Result<void> result) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    result.get();  // just make sure no exception is thrown.
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

template<class type>
void test_ready_result(::plain::concurrency::result::Shared<type> result) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    ASSERT_EQ(result.get(), value_gen<type>::default_value());
  } catch (...) {
    ASSERT_TRUE(false);
  }

  test_same_ref_shared_result(result);
}

template<>
inline void test_ready_result(
  ::plain::concurrency::result::Shared<void> result) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Value);

  try {
    result.get();  // just make sure no exception is thrown.
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

template<class type>
void test_ready_result_custom_exception(
  plain::concurrency::Result<type> result, const intptr_t id) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Exception);

  try {
    result.get();
  } catch (const custom_exception &e) {
    ASSERT_EQ(e.id, id);
    return;
  } catch (const std::runtime_error &e) {
    return;
  } catch (...) {
  }

  ASSERT_TRUE(false);
}

template<class type>
void test_ready_result_custom_exception(
  plain::concurrency::result::Shared<type> result, const intptr_t id) {
  ASSERT_TRUE(static_cast<bool>(result));
  ASSERT_EQ(result.status(), plain::concurrency::ResultStatus::Exception);

  for (size_t i = 0; i < 10; i++) {
    try {
      result.get();
    } catch (const custom_exception &e) {
      ASSERT_EQ(e.id, id);
      if (i == 9) {
        return;
      }
      return;
    } catch (const std::runtime_error &e) {
      UNUSED(e);
      return;
    } catch (...) {
    }
  }

  ASSERT_TRUE(false);
}
}  // namespace plain::tests

namespace plain::tests {
template<class type, class consumer_type>
void test_result_array(
  std::vector<concurrency::Result<type>> results, consumer_type &&consumer,
  value_gen<type> converter) {
  for (size_t i = 0; i < results.size(); i++) {
    if constexpr (!std::is_same_v<void, type>) {
      test_ready_result(consumer(std::move(results[i])), converter.value_of(i));
    } else {
      test_ready_result(consumer(std::move(results[i])));
    }
  }
}

template<class type, class consumer_type>
void test_exceptional_array(
  std::vector<concurrency::Result<type>> results, consumer_type &&consumer) {
  for (size_t i = 0; i < results.size(); i++) {
    test_ready_result_custom_exception(consumer(std::move(results[i])), i);
  }
}

template<class type, class consumer_type>
void test_shared_result_array(
  std::vector<concurrency::result::Shared<type>> results,
  consumer_type &&consumer, value_gen<type> converter) {
  for (size_t i = 0; i < results.size(); i++) {
    if constexpr (!std::is_same_v<void, type>) {
      test_ready_result(consumer(std::move(results[i])), converter.value_of(i));
    } else {
      test_ready_result(consumer(std::move(results[i])));
    }
  }
}

template<class type, class consumer_type>
void test_shared_result_exceptional_array(
  std::vector<concurrency::result::Shared<type>> results,
  consumer_type &&consumer) {
  for (size_t i = 0; i < results.size(); i++) {
    test_ready_result_custom_exception(consumer(std::move(results[i])), i);
  }
}

}  // namespace plain::tests

#endif
