/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id producer_context.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:16
 * @uses The concurrency result producer context detail.
 */

#ifndef PLAIN_CONCURRENCY_RESULT_DETAIL_PRODUCER_CONTEXT_H_
#define PLAIN_CONCURRENCY_RESULT_DETAIL_PRODUCER_CONTEXT_H_

#include "plain/concurrency/result/detail/config.h"
#include <cassert>
#include <utility>
#include "plain/basic/logger.h"

namespace plain::concurrency {
namespace result::detail {

template <typename T>
class ProducerContext {

 public:
  ~ProducerContext() noexcept {
    switch (status_) {
      case ResultStatus::Value: {
        storage_.object.~T();
        break;
      }
      case ResultStatus::Exception: {
        storage_.exception.~exception_ptr();
        break;
      }
      case ResultStatus::Idle: {
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
  }

 public:
  template <typename ...Args>
  void build_result(Args &&... args)
    noexcept(noexcept(T(std::forward<Args>(args)...))) {
    assert(status_ == ResultStatus::Idle);
    new (std::addressof(storage_.object)) T(std::forward<Args>(args)...);
    status_ = ResultStatus::Value;
  }

  void build_exception(const std::exception_ptr& exception) noexcept {
    assert(status_ == ResultStatus::Idle);
    new (std::addressof(storage_.exception)) std::exception_ptr(exception);
    status_ = ResultStatus::Exception;
  }

  ResultStatus status() const noexcept {
    return status_;
  }

  T get() {
    return std::move(get_ref());
  }

  T &get_ref() {
    assert(status_ != ResultStatus::Idle);
    if (status_ == ResultStatus::Value)
      return storage_.object;
    assert(status_ == ResultStatus::Exception);
    assert(static_cast<bool>(storage_.exception));
    std::rethrow_exception(storage_.exception);
  }

 private:
  union storage {
    T object;
    std::exception_ptr exception;
    storage() {}
    ~storage() {}
  };

 private:
  storage storage_;
  ResultStatus status_{ResultStatus::Idle};

};


template <>
class ProducerContext<void> {

 public:
  ~ProducerContext() noexcept {
    if (status_ == ResultStatus::Exception)
      storage_.exception.~exception_ptr();
  }

  ProducerContext &operator=(ProducerContext &&rhs) noexcept {
    assert(status_ == ResultStatus::Idle);
    status_ = std::exchange(rhs.status_, ResultStatus::Idle);
    if (status_ == ResultStatus::Exception) {
      new (std::addressof(storage_.exception)) std::exception_ptr(
        rhs.storage_.exception);
      rhs.storage_.exception.~exception_ptr();
    }
    return *this;
  }

  void build_result() noexcept {
    assert(status_ == ResultStatus::Idle);
    status_ = ResultStatus::Value;
  }

  void build_exception(const std::exception_ptr& exception) noexcept {
    assert(status_ == ResultStatus::Idle);
    new (std::addressof(storage_.exception)) std::exception_ptr(exception);
    status_ = ResultStatus::Exception;
  }

  ResultStatus status() const noexcept {
    return status_;
  }

  void get() const {
    get_ref();
  }

  void get_ref() const {
    assert(status_ != ResultStatus::Idle);
    if (static_cast<bool>(storage_.exception)) {
      std::rethrow_exception(storage_.exception);
    }
  }

 private:
  union storage {
    std::exception_ptr exception;
    storage() {}
    ~storage() {}
  };

 private:
  storage storage_;
  ResultStatus status_{ResultStatus::Idle};

};

template <typename T>
class ProducerContext<T &> {

 public:
  ~ProducerContext() noexcept {
    if (status_ == ResultStatus::Exception) {
      storage_.exception.~exception_ptr();
    }
  }

 public:
  ProducerContext &operator=(ProducerContext &&rhs) {
    assert(status_ == ResultStatus::Idle);
    status_ = std::exchange(rhs.status_, ResultStatus::Idle);
    switch (status_) {
      case ResultStatus::Value: {
          storage_.pointer = rhs.storage_.pointer;
          break;
      }
      case ResultStatus::Exception: {
        new (std::addressof(storage_.exception)) std::exception_ptr(
          rhs.storage_.exception);
        rhs.storage_.exception.~exception_ptr();
        break;
      }
      case ResultStatus::Idle: {
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
    return *this;
  }

 public:
  void build_result(T &reference) noexcept {
    assert(status_ == ResultStatus::Idle);
    auto pointer = std::addressof(reference);
    assert(pointer != nullptr);
    assert(reinterpret_cast<size_t>(pointer) % alignof(T) == 0);
    storage_.pointer = pointer;
    status_ = ResultStatus::Value;
  }

  void build_exception(std::exception_ptr &exception) {
    assert(status_ == ResultStatus::Idle);
    new (std::addressof(storage_.exception)) std::exception_ptr(exception);
    status_ = ResultStatus::Exception;
  }

  ResultStatus status() const noexcept {
    return status_;
  }

  T &get() const {
    return get_ref();
  }

  T &get_ref() {
    assert(status_ != ResultStatus::Idle);
    if (status_ == ResultStatus::Value) {
      assert(storage_.pointer != nullptr);
      assert(reinterpret_cast<std::size_t>(storage_.pointer) % alignof(T) == 0);
      return *storage_.pointer;
    }
    assert(status_ == ResultStatus::Exception);
    assert(static_cast<bool>(storage_.exception));
    std::rethrow_exception(storage_.exception);
  }

 private:
  union storage {
    T *pointer;
    std::exception_ptr exception;
    storage() {}
    ~storage() {}
  };

 private:
  storage storage_;
  ResultStatus status_{ResultStatus::Idle};

};

} // namespace result::detail
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_RESULT_DETAIL_PRODUCER_CONTEXT_H_
