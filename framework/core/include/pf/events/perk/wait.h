/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id wait.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/15 14:15
 * @uses The wait perk class.
 */

#ifndef PF_EVENTS_PERK_WAIT_H_
#define PF_EVENTS_PERK_WAIT_H_

#include "pf/events/perk/config.h"
#include "pf/events/perk/basic.h"

namespace pf_events {

namespace perk {

class PF_API Wait : public Basic {

 public:

   // return true when events are waiting in bus.
   bool wait();

   // return true when events are waiting in bus.
   bool wait_for(std::chrono::milliseconds timeout);

   flag_t on_postpone_event(const PostponeHelper &call);

 private:

   std::condition_variable event_waiting_;
   std::mutex wait_mutex_;
   bool has_events_ = false;

};

} // namespace perk

} // namespace pf_events

#endif // PF_EVENTS_PERK_WAIT_H_
