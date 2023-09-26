#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"

namespace plain::tests {

template<class type>
void test_result_constructor_impl();
void test_result_constructor();

template<class type>
void test_result_status_impl();
void test_result_status();

template<class type>
void test_result_get_impl();
void test_result_get();

template<class type>
void test_result_wait_impl();
void test_result_wait();

template<class type>
void test_result_wait_for_impl();
void test_result_wait_for();

template<class type>
void test_result_wait_until_impl();
void test_result_wait_until();

template<class type>
void test_result_assignment_operator_empty_to_empty();
template<class type>
void test_result_assignment_operator_non_empty_to_non_empty();
template<class type>
void test_result_assignment_operator_empty_to_non_empty();
template<class type>
void test_result_assignment_operator_non_empty_to_empty();
template<class type>
void test_result_assignment_operator_assign_to_self();
template<class type>
void test_result_assignment_operator_impl();
void test_result_assignment_operator();

}  // namespace plain::tests

using namespace std::chrono;
using namespace plain::tests;

template<class type>
void plain::tests::test_result_constructor_impl() {
  concurrency::Result<type> default_constructed_result;
  ASSERT_FALSE(static_cast<bool>(default_constructed_result));

  concurrency::ResultPromise<type> rp;
  auto rp_result = rp.get_result();
  ASSERT_TRUE(static_cast<bool>(rp_result));
  ASSERT_EQ(rp_result.status(), concurrency::ResultStatus::Idle);

  auto new_result = std::move(rp_result);
  ASSERT_FALSE(static_cast<bool>(rp_result));
  ASSERT_TRUE(static_cast<bool>(new_result));
  ASSERT_EQ(new_result.status(), concurrency::ResultStatus::Idle);
}

void plain::tests::test_result_constructor() {
  test_result_constructor_impl<int>();
  test_result_constructor_impl<std::string>();
  test_result_constructor_impl<void>();
  test_result_constructor_impl<int&>();
  test_result_constructor_impl<std::string&>();
}

template<class type>
void plain::tests::test_result_status_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        concurrency::Result<type>().status();
      },
      "status - result is empty.");
  }

  // idle result
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    ASSERT_EQ(result.status(), concurrency::ResultStatus::Idle);
  }

  // ready by value
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    rp.set_from_function(value_gen<type>::default_value);
    ASSERT_EQ(result.status(), concurrency::ResultStatus::Value);
  }

  // exception result
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    rp.set_from_function(value_gen<type>::throw_ex);
    ASSERT_EQ(result.status(), concurrency::ResultStatus::Exception);
  }

  // multiple calls of status are ok
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    rp.set_from_function(value_gen<type>::default_value);

    for (size_t i = 0; i < 10; i++) {
      ASSERT_EQ(result.status(), concurrency::ResultStatus::Value);
    }
  }
}

void plain::tests::test_result_status() {
  test_result_status_impl<int>();
  test_result_status_impl<std::string>();
  test_result_status_impl<void>();
  test_result_status_impl<int&>();
  test_result_status_impl<std::string&>();
}

namespace plain::tests {
  template<class type>
  concurrency::Result<type> get_helper(concurrency::Result<type> &res) {
    co_return res.get();
  }
}  // namespace plain::tests

template<class type>
void plain::tests::test_result_get_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        concurrency::Result<type>().get();
      },
      "get - result is empty.");
  }

  // get blocks until value is present and empties the result
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    auto done_result = get_helper(result);
    const auto now = high_resolution_clock::now();

    ASSERT_FALSE(static_cast<bool>(result));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    test_ready_result(std::move(done_result));
    thread.join();
  }

  // get blocks until exception is present and empties the result
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto id = 12345689;
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), id, unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    auto done_result = get_helper(result);
    const auto now = high_resolution_clock::now();

    ASSERT_FALSE(static_cast<bool>(result));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    test_ready_result_custom_exception(std::move(done_result), id);

    thread.join();
  }

  // if result is ready with value, get returns immediately
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    rp.set_from_function(value_gen<type>::default_value);

    const auto time_before = high_resolution_clock::now();
    auto done_result = get_helper(result);
    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time =
      duration_cast<milliseconds>(time_after - time_before).count();
    ASSERT_FALSE(static_cast<bool>(result));
    ASSERT_LE(total_blocking_time, 5);
    test_ready_result(std::move(done_result));
  }

  // if result is ready with exception, get returns immediately
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto time_before = high_resolution_clock::now();
    auto done_result = get_helper(result);
    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time =
      duration_cast<milliseconds>(time_after - time_before).count();

    ASSERT_LE(total_blocking_time, 5);
    ASSERT_FALSE(static_cast<bool>(result));
    test_ready_result_custom_exception(std::move(done_result), id);
  }
}

void plain::tests::test_result_get() {
  test_result_get_impl<int>();
  test_result_get_impl<std::string>();
  test_result_get_impl<void>();
  test_result_get_impl<int&>();
  test_result_get_impl<std::string&>();
}

template<class type>
void plain::tests::test_result_wait_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        concurrency::Result<type>().wait();
      },
      "wait - result is empty.");
  }

  // wait blocks until value is present
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    result.wait();
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    test_ready_result(std::move(result));
    thread.join();
  }

  // wait blocks until exception is present
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto id = 123456789;
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), id, unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    result.wait();
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    test_ready_result_custom_exception(std::move(result), id);
    thread.join();
  }

  // if result is ready with value, wait returns immediately
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    rp.set_from_function(value_gen<type>::default_value);

    const auto time_before = high_resolution_clock::now();
    result.wait();
    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time = 
      duration_cast<milliseconds>(time_after - time_before).count();

    ASSERT_LE(total_blocking_time, 5);
    test_ready_result(std::move(result));
  }

  // if result is ready with exception, wait returns immediately
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto time_before = high_resolution_clock::now();
    result.wait();
    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time = 
      duration_cast<milliseconds>(time_after - time_before).count();

    ASSERT_LE(total_blocking_time, 5);
    test_ready_result_custom_exception(std::move(result), id);
  }

  // multiple calls to wait are ok
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(50);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    for (size_t i = 0; i < 10; i++) {
      result.wait();
    }

    test_ready_result(std::move(result));
    thread.join();
  }
}

void plain::tests::test_result_wait() {
  test_result_wait_impl<int>();
  test_result_wait_impl<std::string>();
  test_result_wait_impl<void>();
  test_result_wait_impl<int&>();
  test_result_wait_impl<std::string&>();
}

template<class type>
void plain::tests::test_result_wait_for_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        concurrency::Result<type>().wait_for(seconds(1));
      },
      "wait_for - result is empty.");
  }

  // if the result is ready by value, don't block and return status::value
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();

    rp.set_from_function(value_gen<type>::default_value);

    const auto before = high_resolution_clock::now();
    const auto status = result.wait_for(seconds(10));
    const auto after = high_resolution_clock::now();
    const auto time = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(time, 5);
    ASSERT_EQ(status, concurrency::ResultStatus::Value);
    test_ready_result(std::move(result));
  }

  // if the result is ready by exception, don't block and return status::exception
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const size_t id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto before = high_resolution_clock::now();
    const auto status = result.wait_for(seconds(10));
    const auto after = high_resolution_clock::now();
    const auto time = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(time, 5);
    ASSERT_EQ(status, concurrency::ResultStatus::Exception);
    test_ready_result_custom_exception(std::move(result), id);
  }

  // if timeout reaches and no value/exception - return status::idle
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();

    const auto waiting_time = milliseconds(50);
    const auto before = high_resolution_clock::now();
    const auto status = result.wait_for(waiting_time);
    const auto after = high_resolution_clock::now();
    const auto time = duration_cast<milliseconds>(after - before);

    ASSERT_EQ(status, concurrency::ResultStatus::Idle);
    ASSERT_GE(time, waiting_time);
  }

  // if result is set before timeout, unblock, and return status::value
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    result.wait_for(seconds(10));
    const auto now = high_resolution_clock::now();

    test_ready_result(std::move(result));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    thread.join();
  }

  // if exception is set before timeout, unblock, and return status::exception
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto id = 123456789;
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time, id]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    result.wait_for(seconds(10));
    const auto now = high_resolution_clock::now();

    test_ready_result_custom_exception(std::move(result), id);
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    thread.join();
  }

  // multiple calls of wait_for are ok
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    for (size_t i = 0; i < 10; i++) {
      result.wait_for(milliseconds(10));
    }

    thread.join();
  }
}

void plain::tests::test_result_wait_for() {
  test_result_wait_for_impl<int>();
  test_result_wait_for_impl<std::string>();
  test_result_wait_for_impl<void>();
  test_result_wait_for_impl<int&>();
  test_result_wait_for_impl<std::string&>();
}

template<class type>
void plain::tests::test_result_wait_until_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        const auto later = high_resolution_clock::now() + seconds(10);
        concurrency::Result<type>().wait_until(later);
      },
      "wait_until - result is empty.");
  }

  // if time_point <= now, the function is equivalent to result::status
  {
    concurrency::ResultPromise<type> rp_idle, rp_val, rp_err;
    concurrency::Result<type> idle_result = rp_idle.get_result(),
      value_result = rp_val.get_result(),
      err_result = rp_err.get_result();

    rp_val.set_from_function(value_gen<type>::default_value);
    rp_err.set_from_function(value_gen<type>::throw_ex);

    const auto now = high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    ASSERT_EQ(idle_result.wait_until(now), plain::concurrency::ResultStatus::Idle);
    ASSERT_EQ(value_result.wait_until(now), plain::concurrency::ResultStatus::Value);
    ASSERT_EQ(err_result.wait_until(now), plain::concurrency::ResultStatus::Exception);
  }

  // if the result is ready by value, don't block and return status::value
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();

    rp.set_from_function(value_gen<type>::default_value);

    const auto later = high_resolution_clock::now() + seconds(10);

    const auto before = high_resolution_clock::now();
    const auto status = result.wait_until(later);
    const auto after = high_resolution_clock::now();

    const auto ms = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(ms, 5);
    ASSERT_EQ(status, concurrency::ResultStatus::Value);
    test_ready_result(std::move(result));
  }

  // if the result is ready by exception, don't block and return status::exception
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const size_t id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto later = high_resolution_clock::now() + seconds(10);

    const auto before = high_resolution_clock::now();
    const auto status = result.wait_until(later);
    const auto after = high_resolution_clock::now();

    const auto time = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(time, 5);
    ASSERT_EQ(status, concurrency::ResultStatus::Exception);
    test_ready_result_custom_exception(std::move(result), id);
  }

  // if timeout reaches and no value/exception - return status::idle
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();

    const auto later = high_resolution_clock::now() + milliseconds(50);
    const auto status = result.wait_until(later);
    const auto now = high_resolution_clock::now();

    ASSERT_EQ(status, concurrency::ResultStatus::Idle);
    ASSERT_GE(now, later);
  }

  // if result is set before timeout, unblock, and return status::value
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);
    const auto later = high_resolution_clock::now() + seconds(10);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    result.wait_until(later);
    const auto now = high_resolution_clock::now();

    test_ready_result(std::move(result));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    thread.join();
  }

  // if exception is set before timeout, unblock, and return status::exception
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto id = 123456789;

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);
    const auto later = high_resolution_clock::now() + seconds(10);

    std::thread thread([rp = std::move(rp), unblocking_time, id]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    result.wait_until(later);
    const auto now = high_resolution_clock::now();

    test_ready_result_custom_exception(std::move(result), id);
    ASSERT_GE(now, unblocking_time);
    ASSERT_LE(now, unblocking_time + seconds(1));
    thread.join();
  }

  // multiple calls to wait_until are ok
  {
    concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    for (size_t i = 0; i < 10; i++) {
      const auto later = high_resolution_clock::now() + milliseconds(50);
      result.wait_until(later);
    }

    thread.join();
  }
}

void plain::tests::test_result_wait_until() {
  test_result_wait_until_impl<int>();
  test_result_wait_until_impl<std::string>();
  test_result_wait_until_impl<void>();
  test_result_wait_until_impl<int&>();
  test_result_wait_until_impl<std::string&>();
}

template<class type>
void plain::tests::test_result_assignment_operator_empty_to_empty() {
  concurrency::Result<type> result_0, result_1;
  result_0 = std::move(result_1);
  ASSERT_FALSE(static_cast<bool>(result_0));
  ASSERT_FALSE(static_cast<bool>(result_1));
}

template<class type>
void plain::tests::test_result_assignment_operator_non_empty_to_non_empty() {
  concurrency::ResultPromise<type> rp_0, rp_1;
  concurrency::Result<type> result_0 = rp_0.get_result(), result_1 = rp_1.get_result();

  result_0 = std::move(result_1);

  ASSERT_FALSE(static_cast<bool>(result_1));
  ASSERT_TRUE(static_cast<bool>(result_0));

  rp_0.set_from_function(value_gen<type>::default_value);
  ASSERT_EQ(result_0.status(), concurrency::ResultStatus::Idle);

  rp_1.set_from_function(value_gen<type>::default_value);
  test_ready_result(std::move(result_0));
}

template<class type>
void plain::tests::test_result_assignment_operator_empty_to_non_empty() {
  concurrency::ResultPromise<type> rp_0;
  concurrency::Result<type> result_0 = rp_0.get_result(), result_1;
  result_0 = std::move(result_1);
  ASSERT_FALSE(static_cast<bool>(result_0));
  ASSERT_FALSE(static_cast<bool>(result_1));
}

template<class type>
void plain::tests::test_result_assignment_operator_non_empty_to_empty() {
  concurrency::ResultPromise<type> rp_1;
  concurrency::Result<type> result_0, result_1 = rp_1.get_result();
  result_0 = std::move(result_1);
  ASSERT_TRUE(static_cast<bool>(result_0));
  ASSERT_FALSE(static_cast<bool>(result_1));

  rp_1.set_from_function(value_gen<type>::default_value);
  test_ready_result(std::move(result_0));
}

template<class type>
void plain::tests::test_result_assignment_operator_assign_to_self() {
  concurrency::Result<type> res0;

  // res0 = std::move(res0);
  ASSERT_FALSE(static_cast<bool>(res0));

  concurrency::ResultPromise<type> rp_1;
  auto res1 = rp_1.get_result();

  // res1 = std::move(res1);
  ASSERT_TRUE(static_cast<bool>(res1));
}

template<class type>
void plain::tests::test_result_assignment_operator_impl() {
  test_result_assignment_operator_empty_to_empty<type>();
  test_result_assignment_operator_non_empty_to_empty<type>();
  test_result_assignment_operator_empty_to_non_empty<type>();
  test_result_assignment_operator_non_empty_to_non_empty<type>();
  test_result_assignment_operator_assign_to_self<type>();
}

void plain::tests::test_result_assignment_operator() {
  test_result_assignment_operator_impl<int>();
  test_result_assignment_operator_impl<std::string>();
  test_result_assignment_operator_impl<void>();
  test_result_assignment_operator_impl<int&>();
  test_result_assignment_operator_impl<std::string&>();
}

using namespace plain::tests;

class Result : public testing::Test {

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

TEST_F(Result, testConstructor) {
  test_result_constructor();
}

TEST_F(Result, testStatus) {
  test_result_status();
}

TEST_F(Result, testGet) {
  test_result_get();
}

TEST_F(Result, testWait) {
  test_result_wait();
}

TEST_F(Result, testWaitFor) {
  test_result_wait_for();
}

TEST_F(Result, testWaitUntil) {
  test_result_wait_until();
}

TEST_F(Result, testOperatorAssignment) {
  test_result_assignment_operator();
}

/*
int main() {
  tester tester("result test");

  tester.add_step("constructor", test_result_constructor);
  tester.add_step("status", test_result_status);
  tester.add_step("get", test_result_get);
  tester.add_step("wait", test_result_wait);
  tester.add_step("wait_for", test_result_wait_for);
  tester.add_step("wait_until", test_result_wait_until);
  tester.add_step("operator =", test_result_assignment_operator);

  tester.launch_test();
  return 0;
}
*/
