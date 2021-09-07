/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id postpone_helper.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/06 13:44
 * @uses The event postpone helper class.
 */

#ifndef PF_EVENTS_POSTPONE_HELPER_H_
#define PF_EVENTS_POSTPONE_HELPER_H_

#include "pf/events/config.h"
#include "pf/basic/any.h"
#include "pf/events/id.h"
#include "pf/events/listener_attorney.h"
#include "pf/events/protected_stream.h"

namespace pf_events {

template <typename Event>
using DefaultStream = ProtectedStream<Event>;
using create_stream_callback_t = 
  std::unique_ptr<pf_interfaces::events::Stream> (*const)();
using postpone_callback_t = 
  bool (*const)(const Bus &bus, const pf_basic::any &event);

template <typename Event>
bool postpone(const Bus &bus, const pf_basic::any &event);

template <typename Event>
std::unique_ptr<pf_interfaces::events::Stream> create_default_stream() {
  return pf_basic::make_unique< DefaultStream<Event> >();
}

class PF_API PostponeHelper {

 public:

   id_t id_ = nullptr;
   pf_basic::any event_;
   postpone_callback_t postpone_callback_ = nullptr;
   create_stream_callback_t create_stream_callback_ = nullptr;

 public:
   
   PostponeHelper(const id_t id,
       pf_basic::any &event,
       postpone_callback_t postpone_callback,
       create_stream_callback_t create_stream_callback
       )
     : id_{id}
     , event_{event}
     , postpone_callback_{postpone_callback}
     , create_stream_callback_{create_stream_callback}
     {}
   
   ~PostponeHelper() = default;

 public:

   template <typename Event>
     static PostponeHelper create(pf_basic::any &&event) {
     return PostponeHelper{get_id<Event>(), 
      std::forward<pf_basic::any>(event), 
      postpone<Event>, 
      create_default_stream<Event>
     };
   }

};

} // namespace pf_events

#endif // PF_EVENTS_POSTPONE_HELPER_H_
