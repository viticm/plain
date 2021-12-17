#include "pf/basic/tinytimer.h"

namespace pf_basic {

TinyTimer::TinyTimer() {
  cleanup();
}

TinyTimer::~TinyTimer() {
  //do nothing
}

bool TinyTimer::isstart() const {
  return isstart_;
}

void TinyTimer::set_termtime(uint64_t time) {
  tick_termtime_ = time;
}

uint64_t TinyTimer::get_termtime() const {
  return tick_termtime_;
}

uint64_t TinyTimer::get_last_ticktime() const {
  return last_ticktime_;
}

void TinyTimer::cleanup() {
  tick_termtime_ = 0;
  isstart_ = false;
  last_ticktime_ = 0;
}

void TinyTimer::start(uint64_t term, uint64_t now) {
  isstart_ = true;
  tick_termtime_ = term;
  last_ticktime_ = now;
}

bool TinyTimer::counting(uint64_t now) {
  if (!isstart_) return false;
  uint64_t new_ticktime = now;
  uint64_t delta = 0;
  if (new_ticktime > last_ticktime_) {
    delta = new_ticktime - last_ticktime_;
  } else {
    if (new_ticktime + 10000 < last_ticktime_) {
      delta = 
        (static_cast<uint64_t>(0xFFFFFFFF) - last_ticktime_) + new_ticktime;
    } else {
      return false;
    }
  }
  if (delta < tick_termtime_) return false;
  last_ticktime_ = new_ticktime;
  return true;
}

} //namespace pf_basic
