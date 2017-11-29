#include "pf/basic/time_manager.h"
#include "pf/basic/io.tcc"
#include "pf/basic/monitor.h"

using namespace pf_basic;

#define gettime() \
  (TIME_MANAGER_POINTER ? TIME_MANAGER_POINTER->get_tickcount() : 0)

monitor::monitor(const std::string &name) : name_{name} {
  start_time_ = gettime();
  io_cdebug("[monitor] (%s) start ...", name_.c_str());
}

monitor::~monitor() {
  auto end_time = gettime();
  io_cdebug(
      "[monitor] (%s) run time: %d", name_.c_str(), end_time - start_time_);
}
