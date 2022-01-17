/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id listener_attorney.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/06 10:40
 * @uses The listener attorney class.
 */

#ifndef PF_EVENTS_LISTENER_ATTORNEY_H_
#define PF_EVENTS_LISTENER_ATTORNEY_H_

#include "pf/events/config.h"
#include "pf/events/id.h"

namespace pf_events {

template <typename EventBus_t>
class ListenerAttorney {

 public:
   template <typename>
   friend class pf_events::Listener;

 private:

   static constexpr uint32_t new_id(EventBus_t& bus) {
     return bus.new_listener_id();
   }

   template <class Event>
   static constexpr void listen(EventBus_t &bus,
       const uint32_t id, std::function<void(const Event &)> &&callback) {
     bus.template _listen<Event>(id, 
         std::forward< std::function< void(const Event &) > >(callback));
   }

   static constexpr void unlisten_all(EventBus_t &bus, const uint32_t id) {
     bus.unlisten_all(id);
   }

   static constexpr void unlisten(EventBus_t &bus,
       const uint32_t listener_id,
       const id_t event_id) {
     bus.unlisten(listener_id, event_id);
   }

};

} // namespace pf_events

#endif // PF_EVENTS_LISTENER_ATTORNEY_H_
