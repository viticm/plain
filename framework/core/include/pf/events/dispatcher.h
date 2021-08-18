/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id dispatcher.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/17 17:44
 * @uses The events dispatcher class.
 */

#ifndef PF_EVENTS_DISPATCHER_H_
#define PF_EVENTS_DISPATCHER_H_

#include "pf/events/config.h"
#include "pf/interfaces/events/dispatcher.h"

namespace pf_events {

class PF_API Dispatcher {

 public:
   Dispatcher() {}
   virtual ~Dispatcher() {}

 

};

} // namespace pf_events

#endif // PF_EVENTS_DISPATCHER_H_
