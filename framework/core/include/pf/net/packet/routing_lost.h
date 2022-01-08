/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id routing_lost.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/12 09:56
 * @uses The to anthor connection routing lost packet.
*/
#ifndef PF_NET_PACKET_ROUTING_LOST_H_
#define PF_NET_PACKET_ROUTING_LOST_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class PF_API RoutingLost : public pf_net::packet::Interface {

 public:
   RoutingLost() : aim_name_{0} {}
   virtual ~RoutingLost() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   virtual uint32_t size() const;
   uint16_t get_id() const { return NET_PACKET_ROUTING_LOST; };
   void set_aim_name(const std::string &aim_name) {
     pf_basic::string::safecopy(
         aim_name_, aim_name.c_str(), sizeof(aim_name_) - 1);
   };

 private:
   char aim_name_[128]; //Connection name.

};

class RoutingLostFactory : public pf_net::packet::Factory {

 public:
   RoutingLostFactory() {}
   virtual ~RoutingLostFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new RoutingLost();
   }
   uint16_t packet_id() const {
     return NET_PACKET_ROUTING_LOST;
   }
   virtual uint32_t packet_max_size() const {
     return 128 + sizeof(uint32_t);
   };

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_ROUTING_LOST_H_
