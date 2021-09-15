/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id tag.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/15 14:05
 * @uses The tag perk class.
 */

#ifndef PF_EVENTS_PERK_TAG_H_
#define PF_EVENTS_PERK_TAG_H_

#include "pf/events/perk/config.h"
#include "pf/events/bus_interface.h"
#include "pf/events/id.h"
#include "pf/basic/any.h"
#include "pf/events/perk/basic.h"

namespace pf_events {

namespace perk {

class PF_API Tag : public Basic {

 public:

   Tag(std::string tag, BusInterface *owner)
     : tag_{tag}, owner_bus_{owner}
   {}

 public:

   flag_t on_pre_postpone_event(const PostponeHelper &call);

   template <typename TagEvent>
   Tag &wrap() {
     static_assert(validate_event<TagEvent>(), "Invalid tag event");
     static_assert(validate_event<typename TagEvent::Event>(), "Invalid event");
     constexpr auto id = get_id<typename TagEvent::Event>();
     events_to_wrap_[id] = [this](pf_basic::any event) {
       TagEvent new_event{tag_, 
          std::move(pf_basic::any_cast<typename TagEvent::Event>(event))};
       owner_bus_->postpone<TagEvent>(std::move(new_event));
     };
     return *this;
   }

 private:

   std::map< id_t, std::function<void(pf_basic::any)> > events_to_wrap_;
   std::string tag_;
   BusInterface *owner_bus_;

};

} // namespace perk

} // namespace pf_events

#endif // PF_EVENTS_PERK_TAG_H_
