/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id monitor.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 14:27
 * @uses The queue monitor interface class.
 */

#ifndef PF_INTERFACES_QUEUE_MONITOR_H_
#define PF_INTERFACES_QUEUE_MONITOR_H_

#include "pf/interfaces/queue/config.h"
#include "pf/basic/type/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API Monitor {

 public:
   Monitor() {}
   virtual ~Monitor() {}

 public:

   using closure_t = pf_basic::type::closure_t;

 public:

   // Register a callback to be executed on every iteration through 
   // the queue loop.
   virtual void looping(closure_t callback) = 0;

   // Register a callback to be executed when a job fails after the 
   // maximum amount of retries.
   virtual void failing(closure_t callback) = 0;

   // Register a callback to be executed when a daemon queue is stopping.
   virtual void stopping(closure_t callback) = 0;

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_MONITOR_H_
