#include "pf/basic/io.tcc"
#include "pf/basic/logger.h"
#include "pf/basic/monitor.h"

using namespace pf_basic;

monitor::monitor(const std::string &name, uint32_t time) : 
  name_{name}, time_{time} {
  start_time_ = std::clock();
  io_cdebug("[monitor] (%s) start ...", name_.c_str());
}

monitor::~monitor() {
  auto end_time = std::clock();
  auto diff_time = 1000.0 * (end_time - start_time_) / CLOCKS_PER_SEC;
  io_cdebug("[monitor] (%s) run time: %.3f", name_.c_str(), diff_time);
  if (time_ > 0 && static_cast<uint32_t>(diff_time) > time_) {
    SLOW_WARNINGLOG(
        "monitor", "[monitor] %s cost %.3f", name_.c_str(), diff_time);
  }
}
