/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/07 00:09
 * @uses The System module config file.
*/
#ifndef PF_SYS_CONFIG_H_
#define PF_SYS_CONFIG_H_

#include "pf/basic/config.h"

namespace pf_sys {

class ThreadCollect;
class ThreadPool;

enum {
  kThreadStatusStop = 0,
  kThreadStatusRun,
};

} //namespace pf_sys

#endif //PF_SYS_CONFIG_H_
