/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id timer.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/13 16:41
 * @uses The engine timer implemention.
 */

#ifndef PLAIN_ENGINE_TIMER_H_
#define PLAIN_ENGINE_TIMER_H_

#include "plain/engine/config.h"
#include "plain/concurrency/config.h"
#include <atomic>
#include <memory>
#include <chrono>

namespace plain {

namespace detail {

class PLAIN_API TimerStateBasic :
  public std::enable_shared_from_this<TimerStateBasic> {

 public:
  using clock_type = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock_type>;
  using milliseconds = std::chrono::milliseconds;

 public:
  TimerStateBasic(
    size_t due_time, size_t frequency,
    std::shared_ptr<concurrency::executor::Basic> executor,
    std::weak_ptr<TimerQueue> timer_queue, bool is_oneshot);
  ~TimerStateBasic() = default;

 public:
  virtual void execute() = 0;

 public:
  void fire();

  bool expired(const time_point now) const noexcept {
    return deadline_ <= now;
  }

  size_t get_frequency() const noexcept {
    return frequency_.load(std::memory_order_relaxed);
  }

  size_t get_due_time() const noexcept {
    return due_time_;
  }

  time_point get_deadline() const noexcept {
    return deadline_;
  }

  bool is_oneshot() const noexcept {
    return is_oneshot_;
  }

  std::shared_ptr<concurrency::executor::Basic> get_executor() const noexcept {
    return executor_;
  }

  std::weak_ptr<TimerQueue> get_timer_queue() const noexcept {
    return timer_queue_;
  }

  void set_new_frequency(size_t new_frequency) noexcept {
    frequency_.store(new_frequency, std::memory_order_relaxed);
  }

  void cancel() noexcept {
    cancelled_.store(true, std::memory_order_relaxed);
  }

  bool cancelled() const noexcept {
    return cancelled_.load(std::memory_order_relaxed);
  }

 private:
  const std::weak_ptr<TimerQueue> timer_queue_;
  const std::shared_ptr<concurrency::executor::Basic> executor_;
  const size_t due_time_;
  std::atomic_size_t frequency_;
  time_point deadline_;
  std::atomic_bool cancelled_;
  const bool is_oneshot_;

 private:
  static time_point make_deadline(milliseconds diff) noexcept {
    return clock_type::now() + diff;
  }

};

template <typename T>
class TimerState : public TimerStateBasic {

 public:
  template <typename CT>
  TimerState(
    size_t due_time, size_t frequency,
    std::shared_ptr<concurrency::executor::Basic> executor,
    std::weak_ptr<TimerQueue> timer_queue, bool is_oneshot, CT &&callable) :
    TimerStateBasic(due_time, frequency, std::move(executor),
      std::move(timer_queue), is_oneshot), callable_{std::move(callable)} {

  }

 public:
  void execute() override {
    if (cancelled()) return;
    callable_();
  }

 private:
  T callable_;

};

} // namespace detail

class PLAIN_API Timer {

 public:
  Timer() noexcept = default;
  ~Timer();
  Timer(std::shared_ptr<detail::TimerStateBasic> timer_impl) noexcept;
  
  Timer(Timer &&rhs) noexcept = default;
  Timer &operator=(Timer &&rhs) noexcept;

  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;

 public:
  explicit operator bool() const noexcept {
    return static_cast<bool>(state_);
  }

 public:
  void cancel();
  std::chrono::milliseconds get_due_time() const;
  std::shared_ptr<concurrency::executor::Basic> get_executor() const;
  std::weak_ptr<TimerQueue> get_timer_queue() const;

  std::chrono::milliseconds get_frequency() const;
  void set_frequency(std::chrono::milliseconds new_frequency);

 private:
  std::shared_ptr<detail::TimerStateBasic> state_;
  void throw_if_empty(const char *message) const;

};

} // namespace plain

#endif // PLAIN_ENGINE_TIMER_H_
