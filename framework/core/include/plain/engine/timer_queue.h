/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id timer_queue.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/07/16 11:24
 * @uses The engine timer queue.
 */

#ifndef PLAIN_ENGINE_TIMER_QUEUE_H_
#define PLAIN_ENGINE_TIMER_QUEUE_H_

#include "plain/engine/config.h"
#include <chrono>
#include <cassert>
#include <mutex>
#include "plain/basic/bind.h"
#include "plain/concurrency/config.h"
#include "plain/engine/timer.h"

namespace plain {

class PLAIN_API TimerQueue : public std::enable_shared_from_this<TimerQueue> {

 public:
  friend class Timer;

 public:
  TimerQueue(
    std::chrono::milliseconds max_waiting_time,
    const std::function<void(std::string_view name)> &started_callback = {},
    const std::function<void(std::string_view name)> &terminated_callback = {});
  ~TimerQueue() noexcept;

 public:
  void shutdown();
  bool shutdown_requested() const noexcept;
  std::chrono::milliseconds max_worker_idle_time() const noexcept;

 public:
  template <typename T, typename ...Args>
  Timer make_timer(
    std::chrono::milliseconds due_time, std::chrono::milliseconds frequency,
    std::shared_ptr<concurrency::executor::Basic> executor,
    T &&callable, Args &&...args) {
    if (!static_cast<bool>(executor))
      throw std::invalid_argument("make_timer - executor is null.");
    return make_timer_impl(
      due_time.count(), frequency.count(), std::move(executor), false,
      bind(std::forward<T>(callable), std::forward<Args>(args)...));
  }

  template <typename T, typename ...Args>
  Timer make_one_shot_timer(
    std::chrono::milliseconds due_time,
    std::shared_ptr<concurrency::executor::Basic> executor,
    T &&callable, Args &&...args) {
    if (!static_cast<bool>(executor))
      throw std::invalid_argument("make_one_shot_timer - executor is null.");
    return make_timer_impl(
      due_time.count(), 0, std::move(executor), true,
      bind(std::forward<T>(callable), std::forward<Args>(args)...));
  }

  concurrency::LazyResult<void> make_delay_object(
    std::chrono::milliseconds due_time,
    std::shared_ptr<concurrency::executor::Basic> executor);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:

  void add_timer(
    std::unique_lock<std::mutex> &lock,
    std::shared_ptr<detail::TimerStateBasic> new_timer);

  template <typename T>
  std::shared_ptr<detail::TimerStateBasic>
  make_timer_impl(
    size_t due_time, size_t frequency,
    std::shared_ptr<concurrency::executor::Basic> executor, bool is_oneshot,
    T &&callable) {
    assert(static_cast<bool>(executor));
    using decayed_type = typename std::decay_t<T>;
    auto timer_state = std::make_shared<detail::TimerState<decayed_type>>(
      due_time, frequency, std::move(executor), weak_from_this(), is_oneshot,
      std::forward<T>(callable));
    {
      std::unique_lock<std::mutex> lock(get_lock());
      add_timer(lock, timer_state);
    }
    return timer_state;
  }

 private:
  void remove_internal_timer(
    std::shared_ptr<detail::TimerStateBasic> existing_timer);

  std::mutex &get_lock();

};

} // namespace plain

#endif // PLAIN_ENGINE_TIMER_QUEUE_H_
