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
#ifndef PF_NET_PACKET_FORWARD_H_
#define PF_NET_PACKET_FORWARD_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class Forward : public pf_net::packet::Interface {

 public:
   Forward() : original_{0} {}
   virtual ~Forward() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   virtual uint32_t size() const;
   uint16_t get_id() const { return NET_PACKET_FORWARD; };
   void set_original(const std::string &original) {
      pf_basic::string::safecopy(
         original_, original.c_str(), sizeof(original_) - 1);
   };
   void set_packet_size(uint32_t _size) {
     packet_size_ = _size;
   };

 private:
   char original_[128];
   uint32_t packet_size_;

};

class ForwardFactory : public pf_net::packet::Factory {

 public:
   ForwardFactory() {}
   virtual ~ForwardFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new Forward();
   }
   uint16_t packet_id() const {
     return NET_PACKET_FORWARD;
   }
   virtual uint32_t packet_max_size() const {
     return NET_PACKET_DYNAMIC_SIZEMAX;
   };

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_FORWARD_H_
