/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id routing_response.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/08 14:53
 * @uses The to anthor connection routing response packet.
*/
#ifndef PF_NET_PACKET_ROUTING_RESPONSE_H_
#define PF_NET_PACKET_ROUTING_RESPONSE_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class RoutingResponse : public pf_net::packet::Interface {

 public:
   RoutingResponse() : aim_name_{0} {}
   virtual ~RoutingResponse() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   virtual uint32_t size() const;
   uint16_t get_id() const { return NET_PACKET_ROUTING_RESPONSE; };
   void set_aim_name(const std::string &aim_name) {
     pf_basic::string::safecopy(
         aim_name_, aim_name.c_str(), sizeof(aim_name_) - 1);
   };

 private:
   char aim_name_[128]; //The connection name.
};

class RoutingResponseFactory : public pf_net::packet::Factory {

 public:
   RoutingResponseFactory() {}
   virtual ~RoutingResponseFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new RoutingResponse();
   }
   uint16_t packet_id() const {
     return NET_PACKET_ROUTING_RESPONSE;
   }
   virtual uint32_t packet_max_size() const {
     return 128 + sizeof(uint32_t);
   };

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_ROUTING_RESPONSE_H_
