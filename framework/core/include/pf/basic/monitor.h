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
   monitor(const std::string &name);
   ~monitor();

 private:
   explicit monitor(monitor &);
   monitor &operator = (monitor &);

 private:
   std::string name_;
   uint32_t start_time_;

};

} //namespace pf_basic

#endif //PF_BASIC_MONITOR_H_
