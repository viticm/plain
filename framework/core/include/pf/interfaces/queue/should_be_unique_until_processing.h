/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id should_be_unique_until_processing.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 16:33
 * @uses The should be unique until processing interface class.
 */

#ifndef PF_INTERFACES_QUEUE_SHOULD_BE_UNIQUE_UNTIL_PROCESSING_H_
#define PF_INTERFACES_QUEUE_SHOULD_BE_UNIQUE_UNTIL_PROCESSING_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API ShouldBeUniqueUntilProcessing {

 public:

   ShouldBeUniqueUntilProcessing() {}
   virtual ~ShouldBeUniqueUntilProcessing() {}

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_SHOULD_BE_UNIQUE_UNTIL_PROCESSING_H_
