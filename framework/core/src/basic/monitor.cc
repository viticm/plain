#include "pf/basic/io.tcc"
#include "pf/basic/monitor.h"

using namespace pf_basic;

monitor::monitor(const std::string &name) : name_{name} {
  start_time_ = std::clock();
  io_cdebug("[monitor] (%s) start ...", name_.c_str());
}

monitor::~monitor() {
  auto end_time = std::clock();
  auto diff_time = 1000.0 * (end_time - start_time_) / CLOCKS_PER_SEC;
  io_cdebug("[monitor] (%s) run time: %.3f", name_.c_str(), diff_time);
}
