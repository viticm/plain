/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id routing.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/08 15:59
 * @uses The to anthor connection routing packet.
*/
#ifndef PF_NET_PACKET_ROUTING_H_
#define PF_NET_PACKET_ROUTING_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class PF_API Routing : public pf_net::packet::Interface {

 public:
   Routing() : destination_{0}, aim_name_{0}, packet_size_{0} {}
   virtual ~Routing() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   virtual uint32_t size() const;
   uint16_t get_id() const { return NET_PACKET_ROUTING; };
   void set_destination(const std::string &destination) {
      pf_basic::string::safecopy(
         destination_, destination.c_str(), sizeof(destination_) - 1);
   };
   void set_aim_name(const std::string &aim_name) {
     pf_basic::string::safecopy(
         aim_name_, aim_name.c_str(), sizeof(aim_name_) - 1);
   };
   void set_packet_size(uint32_t _size) {
     packet_size_ = _size;
   };

 private:
   char destination_[128];
   char aim_name_[128];
   uint32_t packet_size_;

};

class RoutingFactory : public pf_net::packet::Factory {

 public:
   RoutingFactory() {}
   virtual ~RoutingFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new Routing();
   }
   uint16_t packet_id() const {
     return NET_PACKET_ROUTING;
   }
   virtual uint32_t packet_max_size() const {
     return NET_PACKET_DYNAMIC_SIZEMAX;
   };

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_ROUTING_H_
