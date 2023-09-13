#include "plain/engine/timer.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/engine/timer_queue.h"

using plain::detail::TimerStateBasic;
using plain::Timer;

TimerStateBasic::TimerStateBasic(
  size_t due_time, size_t frequency,
  std::shared_ptr<concurrency::executor::Basic> executor,
  std::weak_ptr<TimerQueue> timer_queue, bool is_oneshot) :
  timer_queue_{std::move(timer_queue)}, executor_{std::move(executor)},
  due_time_{due_time}, frequency_{frequency},
  deadline_{make_deadline(milliseconds(due_time))},
  cancelled_{false}, is_oneshot_{is_oneshot} {

}

void TimerStateBasic::fire() {
  const auto frequency = frequency_.load(std::memory_order_relaxed);
  deadline_ = make_deadline(milliseconds(frequency));

  assert(static_cast<bool>(executor_));

  executor_->post([self = shared_from_this()]() mutable {
    self->execute();
  });
}

Timer::Timer(std::shared_ptr<TimerStateBasic> timer_impl) noexcept :
  state_{std::move(timer_impl)} {

}

Timer::~Timer() noexcept {
  cancel();
}

void Timer::throw_if_empty(const char *error_message) const {
  if (static_cast<bool>(state_)) return;
  throw std::runtime_error(error_message);
}

std::chrono::milliseconds Timer::get_due_time() const {
  throw_if_empty("timer is empty");
  return std::chrono::milliseconds(state_->get_due_time());
}

std::chrono::milliseconds Timer::get_frequency() const {
  throw_if_empty("timer is empty");
  return std::chrono::milliseconds(state_->get_frequency());
}

std::shared_ptr<plain::concurrency::executor::Basic> 
Timer::get_executor() const {
  throw_if_empty("timer is empty");
  return state_->get_executor();
}

std::weak_ptr<plain::TimerQueue> Timer::get_timer_queue() const {
  throw_if_empty("timer is empty");
  return state_->get_timer_queue();
}

void Timer::cancel() {
  if (!static_cast<bool>(state_))
    return;

  auto state = std::move(state_);
  state->cancel();

  auto timer_queue = state->get_timer_queue().lock();

  if (!static_cast<bool>(timer_queue))
    return;
  timer_queue->remove_internal_timer(std::move(state));
}

void Timer::set_frequency(std::chrono::milliseconds new_frequency) {
  throw_if_empty("timer is empty");
  return state_->set_new_frequency(new_frequency.count());
}

Timer &Timer::operator=(Timer &&rhs) noexcept {
  if (this == &rhs)
    return *this;

  if (static_cast<bool>(*this))
    cancel();
  
  state_ = std::move(rhs.state_);
  return *this;
}
