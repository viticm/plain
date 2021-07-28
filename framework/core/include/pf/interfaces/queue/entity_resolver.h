/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id entity_resolver.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 17:34
 * @uses The queue entity resolver interface class.
 */

#ifndef PF_INTERFACES_QUEUE_ENTITY_RESOLVER_H_
#define PF_INTERFACES_QUEUE_ENTITY_RESOLVER_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API EntityResolver {

 public:

   EntityResolver() {}
   virtual ~EntityResolver() {}

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_ENTITY_RESOLVER_H_
