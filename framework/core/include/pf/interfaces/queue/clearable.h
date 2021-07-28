/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id clearable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 14:14
 * @uses The clearable queue interface class.
 */

#ifndef PF_INTERFACES_QUEUE_CLEARABLE_H_
#define PF_INTERFACES_QUEUE_CLEARABLE_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API Clearable {

 public:

   Clearable() {}
   virtual ~Clearable() {}

 public:

   // Delete all of the jobs from the queue.
   virtual void clear(const std::string &queue) = 0;

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_CLEARABLE_H_
