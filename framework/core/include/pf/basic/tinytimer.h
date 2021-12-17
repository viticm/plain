/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id tinytimer.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2014/08/11 19:06
 * @uses tiny timer class
 */
#ifndef PF_BASIC_TINYTIMER_H_
#define PF_BASIC_TINYTIMER_H_

#include "pf/basic/config.h"

namespace pf_basic {

class PF_API TinyTimer {

 public:
   TinyTimer();
   ~TinyTimer();

 public:
   bool isstart() const;
   void set_termtime(uint64_t time);
   uint64_t get_termtime() const;
   uint64_t get_last_ticktime() const;
   void cleanup();
   void start(uint64_t term, uint64_t now);
   bool counting(uint64_t time);

 private:
   uint64_t tick_termtime_;
   uint64_t last_ticktime_;
   bool isstart_;

};

} //namespace pf_basic

#endif //PF_BASIC_TINYTIMER_H_
