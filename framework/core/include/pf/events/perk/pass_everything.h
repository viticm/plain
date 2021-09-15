/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id pass_everything.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/15 10:36
 * @uses The pass everything perk class.
 */

#ifndef PF_EVENTS_PERK_PASS_EVERYTHING_H_
#define PF_EVENTS_PERK_PASS_EVERYTHING_H_

#include "pf/events/perk/config.h"
#include "pf/events/bus_interface.h"
#include "pf/events/perk/basic.h"

namespace pf_events {

namespace perk {

class PF_API PassEverything : public Basic {

 public:

   PassEverything(std::shared_ptr<pf_events::BusInterface> to)
     : pass_to_bus_{std::move(to)}
   {}

   flag_t on_pre_postpone_event(const PostponeHelper &call);

 private:

   std::shared_ptr<pf_events::BusInterface> pass_to_bus_;

};

} // namespace perk

} // namespace pf_events


#endif // PF_EVENTS_PERK_PASS_EVERYTHING_H_
