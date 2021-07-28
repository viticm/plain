/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id should_be_encrypted.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/07/28 17:15
 * @uses The should be encrypted interface class.
 */

#ifndef PF_INTERFACES_QUEUE_SHOULD_BE_ENCRYPTED_H_
#define PF_INTERFACES_QUEUE_SHOULD_BE_ENCRYPTED_H_

#include "pf/interfaces/queue/config.h"

namespace pf_interfaces {

namespace queue {

class PF_API ShouldBeEncrypted {

 public:
   ShouldBeEncrypted() {}
   virtual ~ShouldBeEncrypted() {}

};

} // namespace queue

} // namespace pf_interfaces

#endif // PF_INTERFACES_QUEUE_SHOULD_BE_ENCRYPTED_H_
