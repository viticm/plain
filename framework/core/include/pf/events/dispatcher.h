/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id dispatcher.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/25 15:17
 * @uses The events dispatcher class.
 */

#ifndef PF_EVENTS_DISPATCHER_H_
#define PF_EVENTS_DISPATCHER_H_

#include "pf/events/config.h"
#include "pf/basic/any.h"
#include "pf/interfaces/queue/interface.h"
#include "pf/interfaces/events/dispatcher.h"

namespace pf_events {

class PF_API Dispatcher : public pf_interfaces::events::Dispatcher {

 public:
   Dispatcher() {}
   virtual ~Dispatcher() {}

 public:
   using any = pf_basic::any;
   using Queue = pf_interfaces::queue::Interface;

 public:

   // Register an event listener with the dispatcher.
   virtual void listen(const any &events, const any &listener = nullptr);

   // Setup a wildcard listener callback.
   void setup_wildcard_listen(const std::string &event, const any &listener);

   // Determine if a given event has listeners.
   virtual bool has_listeners(const std::string &name);

   // Determine if the given event has any wildcard listeners.
   bool has_wildcard_listeners(const std::string &name);

   // Register an event and payload to be fired later.
   virtual void push(const std::string &event,
       const std::vector<std::string> &payload = {});

   // Flush a set of pushed events.
   virtual void flush(const std::string &event);

   // Register an event subscriber with the dispatcher.
   virtual void subscribe(const any &subscriber);

   // Resolve the subscriber instance.
   any resolve_subscriber(const any &subscriber);

   // Fire an event until the first non-null response is returned.
   any until(const any &event, const std::vector<std::string> &payload = {});

   // Fire an event and call the listeners.
   virtual void dispatch(const any &event,
       const std::vector<std::string> &payload = {},
       bool halt = false);

 protected:

   // Parse the given event and payload and prepare them for dispatching.
   std::vector<any> parse_event_and_payload(const any &event,
       const std::vector<std::string> &payload);

   // Determine if the payload has a broadcastable event.
   bool should_broadcast(const std::vector<std::string> &payload);

   // Check if event should be broadcasted by condition.
   bool broadcast_when(const any &event);

   // Broadcast the given event class.
   void broadcast_event();

   // Get all of the listeners for a given event name.
   std::vector<std::string> get_listeners(const std::string &name);

   // Get the wildcard listeners for the event.
   std::vector<std::string> get_wildcard_listeners(const std::string &name);

   // Add the listeners for the event's interfaces to the given array.
   std::vector<std::string> add_interface_listeners(
       const std::string &name, const std::vector<any> &listeners);

   // Register an event listener with the dispatcher.
   any make_listener(const any &listener, bool wildcard = false);

   // Create a class based listener using the IoC container.
   any create_class_listener(const std::string &listener, bool wildcard = false);

   // Create the class based event callable.
   any create_class_callable(const any &listener);

   // Parse the class listener into class and method.
   std::vector<std::string> parse_class_ballable(const std::string &_class);

   // Determine if the event handler class should be queued.
   bool handler_should_be_queued(const std::string &_class);

   // Create a callable for putting an event handler on the queue.
   any create_queued_handler_callable(
       const std::string &_class, const std::string &method);

   // Determine if the given event handler should be dispatched after 
   // all database transactions have committed.
   bool handler_should_be_dispatched_after_database_transactions(
       const any &listener);

   // Create a callable for dispatching a listener after database transactions.
   any create_callback_for_listener_running_after_commits(
       const any &listener, const std::string &method);

   // Determine if the event handler wants to be queued.
   bool handler_wants_to_be_queued(
       const std::string &_class, std::vector<std::string> &arguments);

   // Queue the handler class.
   void queue_handler(
       const std::string &_class, 
       const std::string &method, 
       const std::vector<std::string> &arguments);

   // Create the listener and job for a queued listener.
   std::vector<any> create_listener_and_job(
       const std::string &_class, 
       const std::string &method, 
       const std::vector<std::string> &arguments);

   // Propagate listener options to the job.
   any propagate_listener_options(const any &listener, const any &job);

   // Remove a set of listeners from the dispatcher.
   virtual void forget(const std::string &event);

   // Forget all of the pushed listeners.
   virtual void forget_pushed();

   // Get the queue implementation from the resolver.
   Queue *resolve_queue();

   // Set the queue resolver implementation.
   Dispatcher &set_queue_resolver(const any &resolver);

};

} // namespace pf_events

#endif // PF_EVENTS_DISPATCHER_H_
