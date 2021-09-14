/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id bus_interface.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/06 11:42
 * @uses The events bus interface class.
 */

#ifndef PF_EVENTS_BUS_INTERFACE_H_
#define PF_EVENTS_BUS_INTERFACE_H_

#include "pf/events/config.h"
#include "pf/events/id.h"
#include "pf/events/postpone_helper.h"
#include "pf/events/listener.h"

namespace pf_events {

class PF_API BusInterface {

 public:

   template <typename>
   friend class ListenerAttorney;

 public:
   using listener_t = Listener<BusInterface>;

 public:

   BusInterface() = default;
   virtual ~BusInterface() = default;

 public:

   virtual size_t process() = 0;

 public:

   template <typename Event>
   bool postpone(Event event) {
     static_assert(validate_event<Event>(), "Invalid event");
     auto call = PostponeHelper::create<Event>(std::move(event));
     return postpone_event(call);
   }

 protected:

   virtual bool postpone_event(const PostponeHelper &call) = 0;
   virtual pf_interfaces::events::Stream *listen(
       uint32_t listener_id, id_t event_id, 
       create_stream_callback_t create_stream_callback) = 0;
   virtual void unlisten_all(uint32_t listener_id) = 0;
   virtual void unlisten(uint32_t listener_id, id_t event_id) = 0;

 private:

   std::atomic<uint32_t> last_id_{0};

 private:

   uint32_t new_listener_id() {
     return ++last_id_;
   }

   template <class Event>
   void _listen(const uint32_t listener_id, 
       std::function<void(const Event &)> &&callback) {
     static_assert(validate_event<Event>(), "Invalid event");
     constexpr auto event_id = get_id<Event>();
     auto *event_stream = 
       listen(listener_id, event_id, create_default_stream<Event>);
     if (!is_null(event_stream)) {
       event_stream->add_listener(
           listener_id, 
           std::forward< std::function<void(const Event &)> >(callback));
     }
   }

};

template <typename Event>
bool postpone(BusInterface &bus, const pf_basic::any &event) {
  return bus.postpone(std::move(pf_basic::any_cast<Event>(event)));
}

} // namespace pf_events

#endif // PF_EVENTS_BUS_INTERFACE_H_
