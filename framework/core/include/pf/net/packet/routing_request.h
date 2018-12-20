/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id routing_request.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/08 13:45
 * @uses The to anthor connection routing request packet.
*/
#ifndef PF_NET_PACKET_ROUTING_REQUEST_H_
#define PF_NET_PACKET_ROUTING_REQUEST_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class RoutingRequest : public pf_net::packet::Interface {

 public:
   RoutingRequest() : destination_{0}, aim_name_{0}, aim_id_{0} {}
   virtual ~RoutingRequest() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   virtual uint32_t size() const;
   uint16_t get_id() const { return NET_PACKET_ROUTING_REQUEST; };
   void set_destination(const std::string &destination) {
      pf_basic::string::safecopy(
         destination_, destination.c_str(), sizeof(destination_) - 1);
   };
   void set_aim_name(const std::string &aim_name) {
     pf_basic::string::safecopy(
         aim_name_, aim_name.c_str(), sizeof(aim_name_) - 1);
   };
   void set_aim_id(uint16_t aim_id) {
     aim_id_ = aim_id;
   };

 private:
   char destination_[128]; //Service name.
   char aim_name_[128]; //Connection name.
   uint16_t aim_id_; //Connection id.

};

class RoutingRequestFactory : public pf_net::packet::Factory {

 public:
   RoutingRequestFactory() {}
   virtual ~RoutingRequestFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new RoutingRequest();
   }
   uint16_t packet_id() const {
     return NET_PACKET_ROUTING_REQUEST;
   }
   virtual uint32_t packet_max_size() const {
     return 128 + 128 + sizeof(uint32_t) * 2 + sizeof(uint16_t);
   };

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_ROUTING_REQUEST_H_
