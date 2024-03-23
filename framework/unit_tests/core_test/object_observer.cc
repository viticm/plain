#include "object_observer.h"
#include "plain/all.h"

#include <mutex>
#include <condition_variable>

namespace plain::tests::detail {

class object_observer_state {

 private:
  mutable std::mutex lock_;
  mutable std::condition_variable condition_;
  std::unordered_map<size_t, size_t> execution_map_;
  size_t destruction_count_;
  size_t execution_count_;

 public:
  object_observer_state() : destruction_count_(0), execution_count_(0) {}

  size_t get_destruction_count() const noexcept {
    std::unique_lock<decltype(lock_)> lock(lock_);
    return destruction_count_;
  }

  size_t get_execution_count() const noexcept {
    std::unique_lock<decltype(lock_)> lock(lock_);
    return execution_count_;
  }

  std::unordered_map<size_t, size_t> get_execution_map() const noexcept {
    std::unique_lock<decltype(lock_)> lock(lock_);
    return execution_map_;
  }

  bool wait_execution_count(size_t count, std::chrono::milliseconds timeout) {
    std::unique_lock<decltype(lock_)> lock(lock_);
    return condition_.wait_for(lock, timeout, [count, this] {
      return count == execution_count_;
    });
  }

  void on_execute() {
    const auto this_id = ::plain::thread::get_current_virtual_id();

    {
      std::unique_lock<decltype(lock_)> lock(lock_);
      ++execution_count_;
      ++execution_map_[this_id];
    }

    condition_.notify_all();
  }

  bool wait_destruction_count(size_t count, std::chrono::milliseconds timeout) {
    std::unique_lock<decltype(lock_)> lock(lock_);
    return condition_.wait_for(lock, timeout, [count, this] {
      return count == destruction_count_;
    });
  }

  void on_destroy() {
    {
      std::unique_lock<decltype(lock_)> lock(lock_);
      ++destruction_count_;
    }

    condition_.notify_all();
  }
};

}  // namespace plain::tests::detail

using plain::tests::testing_stub;
using plain::tests::object_observer;
using plain::tests::value_testing_stub;

testing_stub& testing_stub::operator=(testing_stub&& rhs) noexcept {
  if (this == &rhs) {
    return *this;
  }

  if (static_cast<bool>(state_)) {
    state_->on_destroy();
  }

  state_ = std::move(rhs.state_);
  return *this;
}

void testing_stub::operator()() noexcept {
  if (static_cast<bool>(state_)) {
    state_->on_execute();
  }
}

value_testing_stub& value_testing_stub::operator=(
  value_testing_stub&& rhs) noexcept {
  testing_stub::operator=(std::move(rhs));
  return *this;
}

size_t value_testing_stub::operator()() noexcept {
  testing_stub::operator()();
  return expected_return_value_;
}

object_observer::object_observer() :
  state_(std::make_shared<detail::object_observer_state>()) {}

testing_stub object_observer::get_testing_stub() noexcept {
  return {state_};
}

value_testing_stub object_observer::get_testing_stub(int value) noexcept {
  return {state_, value};
}

value_testing_stub object_observer::get_testing_stub(size_t value) noexcept {
  return {state_, value};
}

bool object_observer::wait_execution_count(
  size_t count, std::chrono::milliseconds timeout) {
  return state_->wait_execution_count(count, timeout);
}

bool object_observer::wait_destruction_count(
  size_t count, std::chrono::milliseconds timeout) {
  return state_->wait_destruction_count(count, timeout);
}

size_t object_observer::get_destruction_count() const noexcept {
  return state_->get_destruction_count();
}

size_t object_observer::get_execution_count() const noexcept {
  return state_->get_execution_count();
}

std::unordered_map<size_t, size_t>
object_observer::get_execution_map() const noexcept {
  return state_->get_execution_map();
}

testing_stub::~testing_stub() noexcept {
  if (static_cast<bool>(state_)) {
    state_->on_destroy();
  }
}
