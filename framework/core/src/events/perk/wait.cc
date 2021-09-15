#include "pf/events/perk/wait.h"

using namespace pf_events::perk;

bool Wait::wait() {
  std::unique_lock<std::mutex> lock(wait_mutex_);
  if (has_events_) {
    has_events_ = false; // reset, assume that processing of events took place
    return true;
  }

  event_waiting_.wait(lock, [this]() { return has_events_;  });

  // At this moment we are still under mutex
  if (has_events_) {
    has_events_ = false; // reset, assume that processing of events took place
    return true;
  }
  return false;
}

bool Wait::wait_for(const std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(wait_mutex_);
  if (has_events_) {
    has_events_ = false;
    return true;
  }
  if (event_waiting_.wait_for(lock, timeout, 
        [this]() { return has_events_; })) {
    has_events_ = false;
    return true;
  }
  return false;
}

flag_t Wait::on_postpone_event(const PostponeHelper&) {
  {
    std::lock_guard<std::mutex> lock(wait_mutex_);
    has_events_ = true;
  }
  event_waiting_.notify_one();
  return flag_t::postpone_continue;
}
