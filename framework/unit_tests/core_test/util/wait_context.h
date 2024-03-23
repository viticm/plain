#ifndef PLAIN_CORE_TEST_WAIT_CONTEXT_H_
#define PLAIN_CORE_TEST_WAIT_CONTEXT_H_

#include "plain/sys/atomic_wait.h"

namespace plain::tests {

class wait_context {

 private:
  std::atomic_int ready_{0};

 public:
  void wait() {
    sys::detail::atomic_wait(ready_, 0, std::memory_order_relaxed);
    assert(ready_.load(std::memory_order_relaxed));
  }

  bool wait_for(size_t milliseconds) {
    const auto res = sys::detail::atomic_wait_for(
      ready_, 0, std::chrono::milliseconds(milliseconds),
      std::memory_order_relaxed);
    return res == sys::detail::AtomicWaitStatus::Success;
  }

  void notify() {
    ready_.store(1, std::memory_order_relaxed);
    sys::detail::atomic_notify_all(ready_);
  }
};

}  // namespace plain::tests

#endif
