/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/07/25 20:19
 * @uses The protocol interface for net.
*/
#ifndef PF_NET_PROTOCOL_INTERFACE_H_
#define PF_NET_PROTOCOL_INTERFACE_H_

#include "pf/net/protocol/config.h"
#include "pf/net/packet/interface.h"
#include "pf/net/connection/config.h"

namespace pf_net {

namespace protocol {

class PF_API Interface {

 public:
   Interface() {};
   virtual ~Interface() {};

 public:
   virtual bool command(connection::Basic *, uint16_t) = 0;
   virtual bool compress(connection::Basic *, char *, char *) = 0;
   virtual bool send(connection::Basic *, packet::Interface *) = 0;
   virtual size_t header_size() const = 0;

};

} //namespace protocol

} //namespace pf_net

#endif //PF_NET_PROTOCOL_INTERFACE_H_
