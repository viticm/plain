/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/03 12:02
 * @uses The System module config file.
*/
#ifndef PLAIN_SYS_CONFIG_H_
#define PLAIN_SYS_CONFIG_H_

#include "plain/basic/config.h"

namespace plain {

class ThreadCollect;
class ThreadPool;

enum {
  kThreadStatusStopped,
  kThreadStatusRunning,
};

} //namespace plain

#endif //PLAIN_SYS_CONFIG_H_
