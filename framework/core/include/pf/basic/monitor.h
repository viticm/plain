/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id monitor.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/05/08 23:25
 * @uses The monitor for all check test.
*/
#ifndef PF_BASIC_MONITOR_H_
#define PF_BASIC_MONITOR_H_

#include "pf/basic/config.h"

namespace pf_basic {

class PF_API monitor {

 public:
   monitor(const std::string &name, uint32_t time = 0);
   ~monitor();

 private:
   explicit monitor(monitor &);
   monitor &operator = (monitor &);

 private:
   std::string name_;
   std::clock_t start_time_;
   uint32_t time_;

};

} //namespace pf_basic

#endif //PF_BASIC_MONITOR_H_
