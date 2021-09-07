/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id noop_stream.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/26 11:48
 * @uses The event noop stream class.
 */

#ifndef PF_EVENTS_NOOP_STREAM_H_
#define PF_EVENTS_NOOP_STREAM_H_

#include "pf/events/config.h"
#include "pf/interfaces/events/stream.h"

namespace pf_events {

class PF_API NoopStream : pf_interfaces::events::Stream {

 public:

   void postpone(const any &event) override {
     throw std::runtime_error("noop");
   }
   
   size_t process(size_t limit) override {
     throw std::runtime_error("noop");
   }
   
   bool add_listener(uint32_t id, const any &callback) override {
     throw std::runtime_error("noop");
   }

   bool remove_listener(uint32_t id) override {
     throw std::runtime_error("noop");
   }

};

} // namespace pf_events

#endif // PF_EVENTS_NOOP_STREAM_H_
