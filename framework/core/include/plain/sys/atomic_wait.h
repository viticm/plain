/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id atomic_wait.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/08/30 15:11
 * @uses The atomic wait implemention.
 */

#ifndef PLAIN_SYS_ATOMIC_WAIT_H_
#define PLAIN_SYS_ATOMIC_WAIT_H_

#include "plain/sys/config.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <cassert>

namespace plain::sys {

namespace detail {

enum class AtomicWaitStatus {
  Success,
  Timeout,
};

void PLAIN_API atomic_wait_native(void *atom, int32_t old) noexcept;
void PLAIN_API atomic_wait_for_native(
  void *atom, int32_t old, std::chrono::milliseconds ms) noexcept;
void PLAIN_API atomic_notify_all_native(void *atom) noexcept;

template <typename T>
void
atomic_wait(std::atomic<T> &atom, T old, std::memory_order order) noexcept {
  static_assert(std::is_standard_layout_v<std::atomic<T>>,
    "std::atom<T> is not standard-layout");
  static_assert(sizeof(T) == sizeof(int32_t), "<T> must be 4 bytes.");
  while (true) {
    const auto val = atom.load(order);
    if (val != old) return;
#if OS_MAC
    atom.wait(old, order);
#else
    atomic_wait_native(&atom, static_cast<int32_t>(old));
#endif
  }
}

template <typename T>
AtomicWaitStatus atomic_wait_for(
  std::atomic<T> &atom, T old, std::chrono::milliseconds ms,
  std::memory_order order) noexcept {
  const auto deadline = std::chrono::system_clock::now() + ms;
#if OS_MAC
  size_t polling_cycle = 0;
#endif
  while (true) {
    if (atom.load(order) != old)
      return AtomicWaitStatus::Success;
    const auto now = std::chrono::system_clock::now();
    if (now >= deadline) {
      if (atom.load(order) != old)
        return AtomicWaitStatus::Success;
      return AtomicWaitStatus::Timeout;
    }
#if OS_MAC
    if (polling_cycle < 64) {
      std::this_thread::yield();
      ++polling_cycle;
      continue;
    }
    if (polling_cycle < 5'000) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      ++polling_cycle;
      continue;
    }
    if (polling_cycle < 10'000) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      ++polling_cycle;
      continue;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    ++polling_cycle;
#else
    const auto time_diff =
      std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    assert(time_diff.count() >= 0);
    atomic_wait_for_native(&atom, static_cast<int32_t>(old), time_diff);
#endif

  }

}

template <typename T>
void atomic_notify_all(std::atomic<T> &atom) noexcept {
#if OS_MAC
  atom.notify_all();
#else
  atomic_notify_all_native(&atom);
#endif
}

} // namespace detail

} // namespace plain::sys

#endif // PLAIN_SYS_ATOMIC_WAIT_H_
