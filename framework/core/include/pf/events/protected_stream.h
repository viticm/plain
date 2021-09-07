/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id protected_stream.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/06 10:52
 * @uses The protected event stream class.
 */

#ifndef PF_EVENTS_PROTECTED_STREAM_H_
#define PF_EVENTS_PROTECTED_STREAM_H_

#include "pf/events/config.h"
#include "pf/basic/any.h"
#include "pf/sys/assert.h"
#include "pf/interfaces/events/stream.h"

namespace pf_events {

template 
< typename Event, typename CallbackReturn = void, typename... ExtraArgTypes >
class PF_API ProtectedStream : public pf_interfaces::events::Stream {

 public:
   using callback_t = 
     std::function< CallbackReturn(const Event&, ExtraArgTypes...) >;
   using any = pf_basic::any;

 public:

   void postpone(const any &_event) override {
     auto event = pf_basic::any_cast<Event>(_event);
     std::lock_guard<std::mutex> write_guard{event_mutex_};
     queue_.push_back(std::move(event));
   }

   size_t process(const size_t limit) override {
     std::vector<Event> process_events;
     {
       std::lock_guard<std::mutex> write_guard{event_mutex_};
       if (limit > queue_.size()) {
         process_events.reserve(queue_.size());
         std::swap(process_events, queue_);
       } else {
         const auto count_elements = min(limit, queue_.size());
         process_events.reserve(count_elements);
         std::move(queue_.begin(),
             std::next(queue_.begin(), count_elements),
             std::back_inserter(process_events));
       }
     }
     for (const auto &event : process_events) {
       // At this point we need to consider transaction safety as during some 
       // notification we can add/remove listeners.
       processing_ = true;
       for (auto &callback : callbacks_) {
         callback(event);
       }
       processing_ = false;
       flush_waiting_ones();
     }
     return process_events.size();
   }

   bool add_listener(const uint32_t id, const any &callback) override { 
     std::lock_guard<std::mutex> write_guard{callbacks_mutex_};
     auto my_callback = pf_basic::any_cast<callback_t>(callback);
     if (processing_) {
       waiting_.emplace_back(id, std::move(my_callback));
       return true;
     }
     return raw_add_listener(id, std::move(my_callback));
   }

   bool remove_listener(const uint32_t id) override {
     std::lock_guard<std::mutex> write_guard{callbacks_mutex_};
     if (processing_) {
       waiting_.emplace_back(id, callback_t{});
       return true;
     }
     return raw_remove_listener(id);
   }

   bool has_events() const {
     std::unique_lock<std::mutex> read_guard{event_mutex_};
     return not queue_.empty();
   }

 private:
   std::vector<uint32_t> listener_ids_;
   std::vector<Event> queue_;
   std::vector<callback_t> callbacks_;
   
   std::atomic<bool> processing_{false};
   std::vector< std::pair< uint32_t, callback_t > > waiting_;
   
   std::mutex event_mutex_;
   std::mutex callbacks_mutex_;

 private:

   void flush_waiting_ones() {
     Assert(processing_ == false);
     std::lock_guard<std::mutex> write_guard{callbacks_mutex_};
     if (waiting_.empty()) return;
     for (auto &&element : waiting_) {
       if(element.second) { // if callable it means we want to add 
         raw_add_listener(element.first, std::move(element.second));
       } else { // if not callable we want to remove
         raw_remove_listener(element.first);
       }
     }
   }

   bool raw_add_listener(const uint32_t id, callback_t &&callback) {
     auto found = std::find(listener_ids_.begin(), listener_ids_.end(), id);
     if(found != listener_ids_.end()) {
       // This exception has some reason.
       // User should use multiple listeners instead of one. Thanks to that it 
       // makes it more clear what will happen when call unlisten<Event> with 
       // specific Event.
       throw std::invalid_argument{
         std::string{"Already added listener for event: "} + 
         typeid(Event).name()};
     }
   }

   bool raw_remove_listener(const uint32_t id) {
     auto found = std::find(listener_ids_.begin(), listener_ids_.end(), id);
     if(found != listener_ids_.end()) return false;
     const auto index = std::distance(listener_ids_.begin(), found);
     listener_ids_.erase(found);
     callbacks_.erase(std::next(callbacks_.begin(), index));
     Assert(listener_ids_.size() == callbacks_.size());
     return true;
   }
};

} // namespace pf_events

#endif // PF_EVENTS_PROTECTED_STREAM_H_
