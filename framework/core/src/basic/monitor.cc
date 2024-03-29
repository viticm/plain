#include "plain/basic/io.h"
#include "plain/basic/logger.h"
#include "plain/basic/monitor.h"

using namespace plain;

monitor::monitor(const std::string &name, uint32_t time) : 
  name_{name}, time_{time} {
  start_time_ = std::clock();
  io_cdebug("[monitor] ({}) start ...", name_);
}

monitor::~monitor() {
  auto end_time = std::clock();
  auto diff_time = 1000.0 * (end_time - start_time_) / CLOCKS_PER_SEC;
  io_cdebug("[monitor] ({}) run time: {}", name_, diff_time);
  if (time_ > 0 && static_cast<uint32_t>(diff_time) > time_) {
    //SLOW_WARNINGLOG(
    //    "monitor", "[monitor] %s cost %.3f", name_.c_str(), diff_time);
    LOG_WARN << name_ << " cost " << diff_time;
  }
}
