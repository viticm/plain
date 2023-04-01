/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id monitor.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/01 17:25
 * @uses The monitor for all check test.
*/
#ifndef PLAIN_BASIC_MONITOR_H_
#define PLAIN_BASIC_MONITOR_H_

#include "plain/basic/config.h"

namespace plain {

class PLAIN_API monitor {

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

} //namespace plain

#endif //PLAIN_BASIC_MONITOR_H_
