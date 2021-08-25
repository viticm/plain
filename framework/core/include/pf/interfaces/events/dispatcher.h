/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id dispatcher.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 09:49
 * @uses The event dispatcher interface class.
 */

#ifndef PF_INTERFACES_EVENTS_DISPATCHER_H_
#define PF_INTERFACES_EVENTS_DISPATCHER_H_

#include "pf/interfaces/events/config.h"
#include "pf/basic/any.h"
#include "pf/basic/type/config.h"

namespace pf_interfaces {

namespace events {

class PF_API Dispatcher {

 public:
   Dispatcher() {}
   virtual ~Dispatcher() {}

 public:
   using any = pf_basic::any;

 public:

   // Register an event listener with the dispatcher.
   virtual void listen(const any &event, 
                       const any &listener = nullptr) = 0;

   // Determine if a given event has listeners.
   virtual bool has_listeners(const std::string &event_name) = 0;

   // Register an event subscriber with the dispatcher.
   virtual void subscribe(const any &subscriber) = 0;

   // Dispatch an event until the first non-null response is returned.
   virtual void dispatch(const any &event,
                         const std::vector<std::string> &payload = {},
                         bool halt = false) = 0;

   // Register an event and payload to be fired later.
   virtual void push(const std::string &event, 
                     const std::vector<std::string> &payload = {}) = 0;

   // Flush a set of pushed events.
   virtual void flush(const std::string &event) = 0;

 protected:

   // Remove a set of listeners from the dispatcher.
   virtual void forget(const std::string &event) = 0;

   // Forget all of the queued listeners.
   virtual void forget_pushed() = 0;

};

} // namespace events

} // namespace pf_interfaces

#endif // PF_INTERFACES_EVENTS_DISPATCHER_H_
