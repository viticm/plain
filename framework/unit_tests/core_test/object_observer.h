#ifndef PLAIN_CORE_TEST_OBJECT_OBSERVER_H_
#define PLAIN_CORE_TEST_OBJECT_OBSERVER_H_

#include <chrono>
#include <cinttypes>
#include <memory>
#include <unordered_map>

namespace plain::tests {

namespace detail {
class object_observer_state;
}

class testing_stub {

 protected:
  std::shared_ptr<detail::object_observer_state> state_;

 public:
  testing_stub() noexcept {}

  testing_stub(std::shared_ptr<detail::object_observer_state> state) noexcept :
    state_(std::move(state)) {}

  testing_stub(testing_stub&& rhs) noexcept = default;

  ~testing_stub() noexcept;

  testing_stub& operator=(testing_stub&& rhs) noexcept;

  void operator()() noexcept;
};

class value_testing_stub : public testing_stub {

 private:
  const size_t expected_return_value_;

 public:
  value_testing_stub(size_t expected_return_value) noexcept :
    expected_return_value_(expected_return_value) {}

  value_testing_stub(
    std::shared_ptr<detail::object_observer_state> state,
    int32_t return_value) noexcept :
    testing_stub(std::move(state)), expected_return_value_(return_value) {}

  value_testing_stub(
    std::shared_ptr<detail::object_observer_state> state,
    size_t return_value) noexcept :
    testing_stub(std::move(state)), expected_return_value_(return_value) {}

  value_testing_stub(value_testing_stub &&rhs) noexcept = default;

  value_testing_stub &operator=(value_testing_stub &&rhs) noexcept;

  size_t operator()() noexcept;
};

class object_observer {
 private:
  const std::shared_ptr<detail::object_observer_state> state_;

 public:
  object_observer();
  object_observer(object_observer &&) noexcept = default;

  testing_stub get_testing_stub() noexcept;
  value_testing_stub get_testing_stub(int32_t value) noexcept;
  value_testing_stub get_testing_stub(size_t value) noexcept;

  bool wait_execution_count(size_t count, std::chrono::milliseconds timeout);
  bool wait_destruction_count(size_t count, std::chrono::milliseconds timeout);

  size_t get_destruction_count() const noexcept;
  size_t get_execution_count() const noexcept;

  std::unordered_map<size_t, size_t> get_execution_map() const noexcept;
};

}  // namespace plain::tests

#endif  // !PLAIN_CORE_TEST_OBJECT_OBSERVER_H_
