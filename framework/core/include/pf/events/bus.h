/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id bus.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/31 16:32
 * @uses The event bus class.
 */

#ifndef PF_EVENTS_BUS_H_
#define PF_EVENTS_BUS_H_

#include "pf/events/config.h"
#include "pf/events/bus_interface.h"

namespace pf_events {

class PF_API Bus : public BusInterface {

 public:

   template <typename>
   friend class ListenerAttorney;

 public:
   
   size_t process_limit(size_t limit);

 public:

   size_t process() override;
   
 protected:

   pf_interfaces::events::Stream *obtain_stream(
       id_t event_id, create_stream_callback_t create_stream_callback);

   pf_interfaces::events::Stream *find_stream(id_t event_id) const;

 protected:

   bool postpone_event(const PostponeHelper &call) override;
   
   void unlisten_all(uint32_t listener_id) override;
   
   pf_interfaces::events::Stream* listen(
       uint32_t listener_id,
       id_t event_id,
       create_stream_callback_t create_stream_callback) override;

   void unlisten(uint32_t listener_id, id_t event_id) override;

 private:
   
   mutable std::mutex streams_mutex_;
   std::mutex process_mutex_;
   std::vector< std::unique_ptr<pf_interfaces::events::Stream> > event_streams_;
   std::map<id_t, pf_interfaces::events::Stream *> event_to_stream_;

 private:

   pf_interfaces::events::Stream *find_stream_unsafe(id_t event_id) const;
   
};

} // namespace pf_events

#endif // PF_EVENTS_BUS_H_
