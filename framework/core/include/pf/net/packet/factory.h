/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id factory.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2013-2013 viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2016/09/13 14:50
 * @uses server and client net packet factory class interface
 */
#ifndef PF_NET_PACKET_FACTORY_H_
#define PF_NET_PACKET_FACTORY_H_

#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class PF_API Factory {

 public:
   virtual ~Factory() {};
   virtual Interface *packet_create() = 0;
   virtual uint16_t packet_id() const = 0;
   virtual uint32_t packet_max_size() const = 0;

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_FACTORY_H_
