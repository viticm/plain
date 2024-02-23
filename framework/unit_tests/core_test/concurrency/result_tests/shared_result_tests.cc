#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/test_ready_result.h"

namespace plain::tests {

template<class type>
void test_shared_result_constructor_impl();
void test_shared_result_constructor();

template<class type>
void test_shared_result_status_impl();
void test_shared_result_status();

template<class type>
void test_shared_result_get_impl();
void test_shared_result_get();

template<class type>
void test_shared_result_wait_impl();
void test_shared_result_wait();

template<class type>
void test_shared_result_wait_for_impl();
void test_shared_result_wait_for();

template<class type>
void test_shared_result_wait_until_impl();
void test_shared_result_wait_until();

template<class type>
void test_shared_result_assignment_operator_empty_to_empty_move();
template<class type>
void test_shared_result_assignment_operator_non_empty_to_non_empty_move();
template<class type>
void test_shared_result_assignment_operator_empty_to_non_empty_move();
template<class type>
void test_shared_result_assignment_operator_non_empty_to_empty_move();
template<class type>
void test_shared_result_assignment_operator_assign_to_self_move();

template<class type>
void test_shared_result_assignment_operator_empty_to_empty_copy();
template<class type>
void test_shared_result_assignment_operator_non_empty_to_non_empty_copy();
template<class type>
void test_shared_result_assignment_operator_empty_to_non_empty_copy();
template<class type>
void test_shared_result_assignment_operator_non_empty_to_empty_copy();
template<class type>
void test_shared_result_assignment_operator_assign_to_self_copy();

template<class type>
void test_shared_result_assignment_operator_impl();
void test_shared_result_assignment_operator();

}  // namespace plain::tests

using namespace std::chrono;
using namespace plain::tests;

template<class type>
void plain::tests::test_shared_result_constructor_impl() {
  plain::concurrency::result::Shared<type> default_constructed_result;
  ASSERT_FALSE(static_cast<bool>(default_constructed_result));

  // from result
  plain::concurrency::ResultPromise<type> rp;
  auto result = rp.get_result();
  plain::concurrency::result::Shared<type> sr(std::move(result));

  ASSERT_TRUE(static_cast<bool>(sr));
  ASSERT_EQ(sr.status(), plain::concurrency::ResultStatus::Idle);

  // copy
  auto copy_result = sr;
  ASSERT_TRUE(static_cast<bool>(copy_result));
  ASSERT_EQ(sr.status(), plain::concurrency::ResultStatus::Idle);

  // move
  auto new_result = std::move(sr);
  ASSERT_FALSE(static_cast<bool>(sr));
  ASSERT_TRUE(static_cast<bool>(new_result));
  ASSERT_EQ(new_result.status(), plain::concurrency::ResultStatus::Idle);
}

void plain::tests::test_shared_result_constructor() {
  test_shared_result_constructor_impl<int>();
  test_shared_result_constructor_impl<std::string>();
  test_shared_result_constructor_impl<void>();
  test_shared_result_constructor_impl<int&>();
  test_shared_result_constructor_impl<std::string&>();
}

template<class type>
void plain::tests::test_shared_result_status_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        plain::concurrency::result::Shared<type>().status();
      },
      "status - result is empty.");
  }

  // idle result
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    ASSERT_EQ(sr.status(), plain::concurrency::ResultStatus::Idle);
  }

  // ready by value
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    rp.set_from_function(value_gen<type>::default_value);
    ASSERT_EQ(sr.status(), plain::concurrency::ResultStatus::Value);
  }

  // exception result
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    rp.set_from_function(value_gen<type>::throw_ex);
    ASSERT_EQ(sr.status(), plain::concurrency::ResultStatus::Exception);
  }

  // multiple calls of status are ok
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    rp.set_from_function(value_gen<type>::default_value);

    for (size_t i = 0; i < 10; i++) {
      ASSERT_EQ(sr.status(), plain::concurrency::ResultStatus::Value);
    }
  }
}

void plain::tests::test_shared_result_status() {
  test_shared_result_status_impl<int>();
  test_shared_result_status_impl<std::string>();
  test_shared_result_status_impl<void>();
  test_shared_result_status_impl<int&>();
  test_shared_result_status_impl<std::string&>();
}

template<class type>
void plain::tests::test_shared_result_get_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        plain::concurrency::result::Shared<type>().get();
      },
      "get - result is empty.");
  }

  // get blocks until value is present
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    sr.get();
    const auto now = high_resolution_clock::now();

    ASSERT_FALSE(static_cast<bool>(result));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    test_ready_result(std::move(sr));
    thread.join();
  }

  // get blocks until exception is present and empties the result
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    const auto id = 12345689;
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), id, unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    try {
      sr.get();
    } catch (...) {
    }

    const auto now = high_resolution_clock::now();

    ASSERT_FALSE(static_cast<bool>(result));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    test_ready_result_custom_exception(std::move(sr), id);

    thread.join();
  }

  // if result is ready with value, get returns immediately
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    rp.set_from_function(value_gen<type>::default_value);

    const auto time_before = high_resolution_clock::now();

    sr.get();

    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time =
      duration_cast<milliseconds>(time_after - time_before).count();
    ASSERT_FALSE(static_cast<bool>(result));
    ASSERT_LE(total_blocking_time, 10);
    test_ready_result(std::move(sr));
  }

  // if result is ready with exception, get returns immediately
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto time_before = high_resolution_clock::now();

    try {
      sr.get();
    } catch (...) {
    }

    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time =
      duration_cast<milliseconds>(time_after - time_before).count();

    ASSERT_LE(total_blocking_time, 10);
    ASSERT_FALSE(static_cast<bool>(result));
    test_ready_result_custom_exception(std::move(sr), id);
  }

  // get can be called multiple times
  {
    plain::concurrency::result::Shared<type> sr_val(result_gen<type>::ready());

    for (size_t i = 0; i < 6; i++) {
      sr_val.get();
      ASSERT_TRUE(static_cast<bool>(sr_val));
    }

    plain::concurrency::result::Shared<type> sr_ex(
      plain::concurrency::result::make_exceptional<type>(
        std::make_exception_ptr(std::exception {})));

    for (size_t i = 0; i < 6; i++) {
      try {
        sr_ex.get();
      } catch (...) {
      }
      ASSERT_TRUE(static_cast<bool>(sr_ex));
    }
  }
}

void plain::tests::test_shared_result_get() {
  test_shared_result_get_impl<int>();
  test_shared_result_get_impl<std::string>();
  test_shared_result_get_impl<void>();
  test_shared_result_get_impl<int&>();
  test_shared_result_get_impl<std::string&>();
}

template<class type>
void plain::tests::test_shared_result_wait_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        plain::concurrency::result::Shared<type>().wait();
      },
      "wait - result is empty.");
  }

  // wait blocks until value is present
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    sr.wait();
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    test_ready_result(std::move(sr));
    thread.join();
  }

  // wait blocks until exception is present
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    const auto id = 123456789;
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), id, unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    sr.wait();
    const auto now = high_resolution_clock::now();

    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));

    test_ready_result_custom_exception(std::move(sr), id);
    thread.join();
  }

  // if result is ready with value, wait returns immediately
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    rp.set_from_function(value_gen<type>::default_value);

    const auto time_before = high_resolution_clock::now();
    sr.wait();
    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time =
      duration_cast<milliseconds>(time_after - time_before).count();

    ASSERT_LE(total_blocking_time, 5);
    test_ready_result(std::move(sr));
  }

  // if result is ready with exception, wait returns immediately
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto time_before = high_resolution_clock::now();
    sr.wait();
    const auto time_after = high_resolution_clock::now();
    const auto total_blocking_time =
      duration_cast<milliseconds>(time_after - time_before).count();

    ASSERT_LE(total_blocking_time, 5);
    test_ready_result_custom_exception(std::move(sr), id);
  }

  // multiple calls to wait are ok
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(50);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    for (size_t i = 0; i < 10; i++) {
      sr.wait();
    }

    test_ready_result(std::move(sr));
    thread.join();
  }
}

void plain::tests::test_shared_result_wait() {
  test_shared_result_wait_impl<int>();
  test_shared_result_wait_impl<std::string>();
  test_shared_result_wait_impl<void>();
  test_shared_result_wait_impl<int&>();
  test_shared_result_wait_impl<std::string&>();
}

template<class type>
void plain::tests::test_shared_result_wait_for_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        plain::concurrency::result::Shared<type>().wait_for(seconds(1));
      },
      "wait_for - result is empty.");
  }

  // if the result is ready by value, don't block and return status::value
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    rp.set_from_function(value_gen<type>::default_value);

    const auto before = high_resolution_clock::now();
    const auto status = sr.wait_for(seconds(10));
    const auto after = high_resolution_clock::now();
    const auto time = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(time, 20);
    ASSERT_EQ(status, plain::concurrency::ResultStatus::Value);
    test_ready_result(std::move(sr));
  }

  // if the result is ready by exception, don't block and return status::exception
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const size_t id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto before = high_resolution_clock::now();
    const auto status = sr.wait_for(seconds(10));
    const auto after = high_resolution_clock::now();
    const auto time = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(time, 20);
    ASSERT_EQ(status, plain::concurrency::ResultStatus::Exception);
    test_ready_result_custom_exception(std::move(sr), id);
  }

  // if timeout reaches and no value/exception - return status::idle
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto waiting_time = milliseconds(50);
    const auto before = high_resolution_clock::now();
    const auto status = sr.wait_for(waiting_time);
    const auto after = high_resolution_clock::now();
    const auto time = duration_cast<milliseconds>(after - before);

    ASSERT_EQ(status, plain::concurrency::ResultStatus::Idle);
    ASSERT_GE(time, waiting_time);
  }

  // if result is set before timeout, unblock, and return status::value
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    sr.wait_for(seconds(10));
    const auto now = high_resolution_clock::now();

    test_ready_result(std::move(sr));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    thread.join();
  }

  // if exception is set before timeout, unblock, and return status::exception
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    const auto id = 123456789;
    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time, id]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    sr.wait_for(seconds(10));
    const auto now = high_resolution_clock::now();

    test_ready_result_custom_exception(std::move(sr), id);
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    thread.join();
  }

  // multiple calls of wait_for are ok
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    for (size_t i = 0; i < 10; i++) {
      sr.wait_for(milliseconds(10));
    }

    thread.join();
  }
}

void plain::tests::test_shared_result_wait_for() {
  test_shared_result_wait_for_impl<int>();
  test_shared_result_wait_for_impl<std::string>();
  test_shared_result_wait_for_impl<void>();
  test_shared_result_wait_for_impl<int&>();
  test_shared_result_wait_for_impl<std::string&>();
}

template<class type>
void plain::tests::test_shared_result_wait_until_impl() {
  // empty result throws
  {
    assert_throws_with_error_message<std::runtime_error>(
      [] {
        const auto later = high_resolution_clock::now() + seconds(10);
        plain::concurrency::result::Shared<type>().wait_until(later);
      },
      "wait_until - result is empty.");
  }

  // if time_point <= now, the function is equivalent to result::status
  {
    plain::concurrency::ResultPromise<type> rp_idle, rp_val, rp_err;
    plain::concurrency::Result<type> idle_result = rp_idle.get_result(),
      value_result = rp_val.get_result(), err_result = rp_err.get_result();
    plain::concurrency::result::Shared<type>
      shared_idle_result(std::move(idle_result)), 
      shared_value_result(std::move(value_result)),
      shared_err_result(std::move(err_result));

    rp_val.set_from_function(value_gen<type>::default_value);
    rp_err.set_from_function(value_gen<type>::throw_ex);

    const auto now = high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    ASSERT_EQ(shared_idle_result.wait_until(now),
      plain::concurrency::ResultStatus::Idle);
    ASSERT_EQ(shared_value_result.wait_until(now),
      plain::concurrency::ResultStatus::Value);
    ASSERT_EQ(shared_err_result.wait_until(now),
      plain::concurrency::ResultStatus::Exception);
  }

  // if the result is ready by value, don't block and return status::value
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    rp.set_from_function(value_gen<type>::default_value);

    const auto later = high_resolution_clock::now() + seconds(10);

    const auto before = high_resolution_clock::now();
    const auto status = sr.wait_until(later);
    const auto after = high_resolution_clock::now();

    const auto ms = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(ms, 20);
    ASSERT_EQ(status, plain::concurrency::ResultStatus::Value);
    test_ready_result(std::move(sr));
  }

  // if the result is ready by exception, don't block and return status::exception
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    const size_t id = 123456789;

    rp.set_exception(std::make_exception_ptr(custom_exception(id)));

    const auto later = high_resolution_clock::now() + seconds(10);

    const auto before = high_resolution_clock::now();
    const auto status = sr.wait_until(later);
    const auto after = high_resolution_clock::now();

    const auto time = duration_cast<milliseconds>(after - before).count();

    ASSERT_LE(time, 20);
    ASSERT_EQ(status, plain::concurrency::ResultStatus::Exception);
    test_ready_result_custom_exception(std::move(sr), id);
  }

  // if timeout reaches and no value/exception - return status::idle
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto later = high_resolution_clock::now() + milliseconds(150);
    const auto status = sr.wait_until(later);
    const auto now = high_resolution_clock::now();

    const auto ms = duration_cast<microseconds>(now - later).count();
    ASSERT_GE(ms, 0); // FIXME: use variable.
    ASSERT_EQ(status, plain::concurrency::ResultStatus::Idle);
    ASSERT_GE(now, later);
  }

  // if result is set before timeout, unblock, and return status::value
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);
    const auto later = high_resolution_clock::now() + seconds(10);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    sr.wait_until(later);
    const auto now = high_resolution_clock::now();

    test_ready_result(std::move(sr));
    ASSERT_GE(now, unblocking_time);
    ASSERT_LT(now, unblocking_time + seconds(1));
    thread.join();
  }

  // if exception is set before timeout, unblock, and return status::exception
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));
    const auto id = 123456789;

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);
    const auto later = high_resolution_clock::now() + seconds(10);

    std::thread thread([rp = std::move(rp), unblocking_time, id]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_exception(std::make_exception_ptr(custom_exception(id)));
    });

    sr.wait_until(later);
    const auto now = high_resolution_clock::now();

    test_ready_result_custom_exception(std::move(sr), id);
    ASSERT_GE(now, unblocking_time);
    ASSERT_LE(now, unblocking_time + seconds(1));
    thread.join();
  }

  // multiple calls to wait_until are ok
  {
    plain::concurrency::ResultPromise<type> rp;
    auto result = rp.get_result();
    plain::concurrency::result::Shared<type> sr(std::move(result));

    const auto unblocking_time = high_resolution_clock::now() + milliseconds(150);

    std::thread thread([rp = std::move(rp), unblocking_time]() mutable {
      std::this_thread::sleep_until(unblocking_time);
      rp.set_from_function(value_gen<type>::default_value);
    });

    for (size_t i = 0; i < 10; i++) {
      const auto later = high_resolution_clock::now() + milliseconds(50);
      sr.wait_until(later);
    }

    thread.join();
  }
}

void plain::tests::test_shared_result_wait_until() {
  test_shared_result_wait_until_impl<int>();
  test_shared_result_wait_until_impl<std::string>();
  test_shared_result_wait_until_impl<void>();
  test_shared_result_wait_until_impl<int&>();
  test_shared_result_wait_until_impl<std::string&>();
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_empty_to_empty_move() {
  plain::concurrency::result::Shared<type> result_0, result_1;
  result_0 = std::move(result_1);
  ASSERT_FALSE(static_cast<bool>(result_0));
  ASSERT_FALSE(static_cast<bool>(result_1));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_non_empty_to_non_empty_move() {
  plain::concurrency::ResultPromise<type> rp_0, rp_1;
  plain::concurrency::Result<type> result_0 = rp_0.get_result(),
    result_1 = rp_1.get_result();
  plain::concurrency::result::Shared<type> sr0(std::move(result_0)),
    sr1(std::move(result_1));

  sr0 = std::move(sr1);

  ASSERT_FALSE(static_cast<bool>(sr1));
  ASSERT_TRUE(static_cast<bool>(sr0));

  rp_0.set_from_function(value_gen<type>::default_value);
  ASSERT_EQ(sr0.status(), plain::concurrency::ResultStatus::Idle);

  rp_1.set_from_function(value_gen<type>::default_value);
  test_ready_result(std::move(sr0));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_empty_to_non_empty_move() {
  plain::concurrency::ResultPromise<type> rp_0;
  plain::concurrency::Result<type> result_0 = rp_0.get_result(), result_1;
  plain::concurrency::result::Shared<type>
    sr0(std::move(result_0)), sr1(std::move(result_1));

  sr0 = std::move(sr1);
  ASSERT_FALSE(static_cast<bool>(sr0));
  ASSERT_FALSE(static_cast<bool>(sr1));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_non_empty_to_empty_move() {
  plain::concurrency::ResultPromise<type> rp_1;
  plain::concurrency::Result<type> result_0, result_1 = rp_1.get_result();
  plain::concurrency::result::Shared<type>
    sr0(std::move(result_0)), sr1(std::move(result_1));

  sr0 = std::move(sr1);
  ASSERT_TRUE(static_cast<bool>(sr0));
  ASSERT_FALSE(static_cast<bool>(sr1));

  rp_1.set_from_function(value_gen<type>::default_value);
  test_ready_result(std::move(sr0));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_assign_to_self_move() {
  plain::concurrency::result::Shared<type> empty;

  // empty = std::move(empty);
  ASSERT_FALSE(static_cast<bool>(empty));

  plain::concurrency::ResultPromise<type> rp_1;
  auto res1 = rp_1.get_result();
  plain::concurrency::result::Shared<type> non_empty(std::move(res1));

  // non_empty = std::move(non_empty);
  ASSERT_TRUE(static_cast<bool>(non_empty));

  auto copy = non_empty;
  copy = std::move(non_empty);
  ASSERT_TRUE(static_cast<bool>(copy));
  ASSERT_TRUE(static_cast<bool>(non_empty));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_empty_to_empty_copy() {
  plain::concurrency::result::Shared<type> result_0, result_1;
  result_0 = result_1;
  ASSERT_FALSE(static_cast<bool>(result_0));
  ASSERT_FALSE(static_cast<bool>(result_1));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_non_empty_to_non_empty_copy() {
  plain::concurrency::ResultPromise<type> rp_0, rp_1;
  plain::concurrency::Result<type>
    result_0 = rp_0.get_result(), result_1 = rp_1.get_result();
  plain::concurrency::result::Shared<type>
    sr0(std::move(result_0)), sr1(std::move(result_1));

  sr0 = sr1;

  ASSERT_TRUE(static_cast<bool>(sr1));
  ASSERT_TRUE(static_cast<bool>(sr0));

  rp_0.set_from_function(value_gen<type>::default_value);
  ASSERT_EQ(sr0.status(), plain::concurrency::ResultStatus::Idle);

  rp_1.set_from_function(value_gen<type>::default_value);
  test_ready_result(std::move(sr0));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_empty_to_non_empty_copy() {
  plain::concurrency::ResultPromise<type> rp_0;
  plain::concurrency::Result<type> result_0 = rp_0.get_result(), result_1;
  plain::concurrency::result::Shared<type>
    sr0(std::move(result_0)), sr1(std::move(result_1));

  sr0 = sr1;
  ASSERT_FALSE(static_cast<bool>(sr0));
  ASSERT_FALSE(static_cast<bool>(sr1));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_non_empty_to_empty_copy() {
  plain::concurrency::ResultPromise<type> rp_1;
  plain::concurrency::Result<type> result_0, result_1 = rp_1.get_result();
  plain::concurrency::result::Shared<type>
    sr0(std::move(result_0)), sr1(std::move(result_1));

  sr0 = sr1;
  ASSERT_TRUE(static_cast<bool>(sr0));
  ASSERT_TRUE(static_cast<bool>(sr1));

  rp_1.set_from_function(value_gen<type>::default_value);
  test_ready_result(std::move(sr0));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_assign_to_self_copy() {
  plain::concurrency::result::Shared<type> empty;

  empty = empty;
  ASSERT_FALSE(static_cast<bool>(empty));

  plain::concurrency::ResultPromise<type> rp_1;
  auto res1 = rp_1.get_result();
  plain::concurrency::result::Shared<type> non_empty(std::move(res1));

  non_empty = non_empty;
  ASSERT_TRUE(static_cast<bool>(non_empty));

  auto copy = non_empty;
  copy = non_empty;
  ASSERT_TRUE(static_cast<bool>(copy));
  ASSERT_TRUE(static_cast<bool>(non_empty));
}

template<class type>
void plain::tests::test_shared_result_assignment_operator_impl() {
  test_shared_result_assignment_operator_empty_to_empty_move<type>();
  test_shared_result_assignment_operator_non_empty_to_empty_move<type>();
  test_shared_result_assignment_operator_empty_to_non_empty_move<type>();
  test_shared_result_assignment_operator_non_empty_to_non_empty_move<type>();
  test_shared_result_assignment_operator_assign_to_self_move<type>();

  test_shared_result_assignment_operator_empty_to_empty_copy<type>();
  test_shared_result_assignment_operator_non_empty_to_empty_copy<type>();
  test_shared_result_assignment_operator_empty_to_non_empty_copy<type>();
  test_shared_result_assignment_operator_non_empty_to_non_empty_copy<type>();
  test_shared_result_assignment_operator_assign_to_self_copy<type>();
}

void plain::tests::test_shared_result_assignment_operator() {
  test_shared_result_assignment_operator_impl<int>();
  test_shared_result_assignment_operator_impl<std::string>();
  test_shared_result_assignment_operator_impl<void>();
  test_shared_result_assignment_operator_impl<int&>();
  test_shared_result_assignment_operator_impl<std::string&>();
}

using namespace plain::tests;

class SharedResult : public testing::Test {

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

TEST_F(SharedResult, testConstructor) {
  test_shared_result_constructor();
}

TEST_F(SharedResult, testStatus) {
  test_shared_result_status();
}

TEST_F(SharedResult, testGet) {
  test_shared_result_get();
}

TEST_F(SharedResult, testWait) {
  test_shared_result_wait();
}

TEST_F(SharedResult, testWaitFor) {
  test_shared_result_wait_for();
}

TEST_F(SharedResult, testWaitUntil) {
  test_shared_result_wait_until();
}

TEST_F(SharedResult, testOperatorAssignment) {
  test_shared_result_assignment_operator();
}

/*
int main() {
  tester tester("shared_result test");

  tester.add_step("constructor", test_shared_result_constructor);
  tester.add_step("status", test_shared_result_status);
  tester.add_step("get", test_shared_result_get);
  tester.add_step("wait", test_shared_result_wait);
  tester.add_step("wait_for", test_shared_result_wait_for);
  tester.add_step("wait_until", test_shared_result_wait_until);
  tester.add_step("operator =", test_shared_result_assignment_operator);

  tester.launch_test();
  return 0;
}
*/
