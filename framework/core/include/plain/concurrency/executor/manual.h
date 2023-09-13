/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id manual.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/05 14:33
 * @uses The concurrency manual executor implemention.
 */

#ifndef PLAIN_CONCURRENCY_EXECUTOR_MANUAL_H_
#define PLAIN_CONCURRENCY_EXECUTOR_MANUAL_H_

#include "plain/concurrency/executor/config.h"
#include "plain/concurrency/executor/derivable.h"
#include <deque>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace plain::concurrency {
namespace executor {

class PLAIN_API alignas(kCacheInlineAlignment)
Manual final : public Derivable<Manual> {

 public:
  Manual();

 public:
  void enqueue(Task task) override;
  void enqueue(std::span<Task> tasks) override;

  int32_t max_concurrency_level() const noexcept override;

  void shutdown() override;
  bool shutdown_requested() const override;

  size_t size() const;
  bool empty() const;

  size_t clear();

  bool loop_once();
  bool loop_once_for(std::chrono::milliseconds max_waiting_time);

  template <typename Clock, typename Duration = typename Clock::duration>
  bool loop_once_until(std::chrono::time_point<Clock, Duration> timeout_time) {
    return loop_until_impl(1, to_system_time_point(timeout_time));
  }

  size_t loop(size_t max_count);
  size_t loop_for(size_t max_count, std::chrono::milliseconds max_waiting_time);

  template <typename Clock, typename Duration = typename Clock::duration>
  size_t loop_until(
    size_t max_count, std::chrono::time_point<Clock, Duration> timeout_time) {
    return loop_until_impl(max_count, to_system_time_point(timeout_time));
  }

  void wait_for_task();
  bool wait_for_task_for(std::chrono::milliseconds max_waiting_time);

  template <typename Clock, typename Duration = typename Clock::duration>
  bool wait_for_task_until(
    std::chrono::time_point<Clock, Duration> timeout_time) {
    return wait_for_tasks_impl(1, to_system_time_point(timeout_time)) == 1;
  }

  void wait_for_tasks(size_t count);
  size_t wait_for_tasks_for(
    size_t count, std::chrono::milliseconds max_waiting_time);

  template <typename Clock, typename Duration = typename Clock::duration>
  size_t wait_for_tasks_until(
    size_t count, std::chrono::time_point<Clock, Duration> timeout_time) {
    return wait_for_tasks_impl(count, to_system_time_point(timeout_time));
  }

 private:
  mutable std::mutex lock_;
  std::deque<Task> tasks_;
  std::condition_variable condition_;
  bool abort_;
  std::atomic_bool atomic_abort_;

 private:
  template <typename Clock, typename Duration = typename Clock::duration>
  static std::chrono::system_clock::time_point
  to_system_time_point(
    const std::chrono::time_point<Clock, Duration> &time_point)
      noexcept(noexcept(Clock::now())) {
    const auto src_now = Clock::now();
    const auto dst_now = std::chrono::system_clock::now();
    return dst_now +
      std::chrono::duration_cast<std::chrono::milliseconds>(
        time_point - src_now);
  }
  
  static std::chrono::system_clock::time_point
  time_point_from_now(std::chrono::milliseconds ms) noexcept {
    return std::chrono::system_clock::now() + ms;
  }

  size_t loop_impl(size_t max_count);
  size_t loop_until_impl(
    size_t max_count,
    std::chrono::time_point<std::chrono::system_clock> deadline);

  void wait_for_tasks_impl(size_t count);
  size_t wait_for_tasks_impl(
    size_t count, std::chrono::time_point<std::chrono::system_clock> deadline);

};

} // namespace executor
} // namespace plain::concurrency

#endif // PLAIN_CONCURRENCY_EXECUTOR_MANUAL_H_
