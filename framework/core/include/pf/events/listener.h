/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id listener.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/31 16:47
 * @uses The event listener class.
 */

#ifndef PF_EVENTS_LISTENER_H_
#define PF_EVENTS_LISTENER_H_

#include "pf/events/config.h"
#include "pf/events/id.h"
#include "pf/events/listener_attorney.h"
#include "pf/events/listener_traits.h"
#include "pf/events/protected_stream.h"

namespace pf_events {

template <class Bus>
class Listener {

 public:
   
   explicit Listener() = default;

   explicit Listener(std::shared_ptr<Bus> bus)
   : id_{ListenerAttorney<Bus>::new_id(*bus)},
   bus_{std::move(bus)} {}

   Listener(const Listener &other) = delete;
   Listener(Listener &&other) = delete;
   ~Listener() {
     if (!is_null(bus_)) unlisten_all();
   }

 public:

   static std::unique_ptr<Listener> create_not_owning(Bus &bus) {
     // return Listener{std::shared_ptr<Bus>{&bus, [](Bus*) {}}};
     return std::unique_ptr<Listener>(
         new Listener(std::shared_ptr<Bus>{&bus, [](Bus *) {}}));
   }

   template <class Event, typename _ = void>
   // constexpr void listen(std::function<void(const Event &)> &&callback) {
   void listen(std::function<void(const Event &)> &&callback) {
     static_assert(validate_event<Event>(), "Invalid event");
     listen_to_callback<Event>(
         std::forward< std::function<void(const Event &)> >(callback));
   }

   template <class Event>
   void listen_to_callback(const std::function<void(const Event &)> &callback) {
     static_assert(validate_event<Event>(), "Invalid event");
     if (is_null(bus_)) throw std::runtime_error{"bus is null"};
     ListenerAttorney<Bus>::template listen<Event>(
         *bus_, id_, std::function<void(const Event &)>{callback}
     );
   }

   template <class Event>
   void listen_to_callback(std::function<void(const Event &)> &&callback) {
     static_assert(validate_event<Event>(), "Invalid event");
     if (is_null(bus_)) throw std::runtime_error{"bus is null"};
     ListenerAttorney<Bus>::template listen<Event>(
         *bus_, id_, std::forward< std::function<void(const Event&)> >(callback)
     ); 
   }

   template <
     class EventCallback, typename Event = first_argument<EventCallback> >
   void listen(EventCallback &&callback) {
     using namespace pf_basic;
     static_assert(std::is_const< remove_reference_t<Event> >::value, 
         "Event should be const");
     static_assert(std::is_reference<Event>::value, 
         "Event should be const & (reference)");
     using PureEvent = remove_const_t< remove_reference_t<Event> >;
     static_assert(validate_event<PureEvent>(), "Invalid event");
     listen_to_callback<PureEvent>(std::forward<EventCallback>(callback));
   }

   void unlisten_all() {
     if (is_null(bus_)) throw std::runtime_error{"bus is null"};
     ListenerAttorney<Bus>::unlisten_all(*bus_, id_);
   }

   template <typename Event>
   void unlisten() {
     static_assert(validate_event<Event>(), "Invalid event");
     if (is_null(bus_)) throw std::runtime_error{"bus is null"};
     ListenerAttorney<Bus>::unlisten(*bus_, id_, get_id<Event>());
   }

   // We want more explicit move so user knows what is going on.
   void transfer(Listener &&from) {
     if (&from == this) throw std::runtime_error("Self transfer not allowed");
     if (!is_null(bus_)) unlisten_all();
     // We don't have to reset listener ID as bus is moved and we won't 
     // call unlisten_all.
     id_ = from.id_;
     bus_ = std::move(from.bus_);
   }

   const std::shared_ptr<Bus> &get_bus() const {
     return bus_;
   }

 private:

   uint32_t id_ = 0;
   std::shared_ptr<Bus> bus_ = nullptr;

};

} // namespace pf_events

#endif // PF_EVENTS_LISTENER_H_
