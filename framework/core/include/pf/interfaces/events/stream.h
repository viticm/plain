/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id stream.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/26 11:27
 * @uses The event stream interface define.
 */

#ifndef PF_INTERFACES_EVENTS_STREAM_H_
#define PF_INTERFACES_EVENTS_STREAM_H_

#include "pf/interfaces/events/config.h"
#include "pf/basic/any.h"

namespace pf_interfaces {

namespace events {

class PF_API Stream {

 public:
   using any = pf_basic::any;

 public:
   virtual ~Stream() = default;
   virtual void postpone(const any &event) = 0;
   virtual size_t process(size_t limit) = 0;
   virtual bool add_listener(uint32_t id, const any &callback) = 0;
   virtual bool remove_listener(uint32_t id) = 0;

};

} // namespace events

} // namespace pf_interfaces

#endif // PF_INTERFACES_EVENTS_STREAM_H_
