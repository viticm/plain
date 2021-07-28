/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id factory.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 13:55
 * @uses The queue factory interface class.
 */

#ifndef PF_INTERFACES_QUEUE_FACTORY_H_
#define PF_INTERFACES_QUEUE_FACTORY_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API Factory {

 public:

   Factory() {}
   virtual ~Factory() {}

 public:
   virtual Interface *connection(const std::string &name = "") = 0;

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_FACTORY_H_
