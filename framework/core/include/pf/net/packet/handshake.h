/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id handshake.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/11/24 15:11
 * @uses The server to client safe handshake packet.
*/
#ifndef PF_NET_PACKET_HANDSHAKE_H_
#define PF_NET_PACKET_HANDSHAKE_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class PF_API Handshake : public pf_net::packet::Interface {

 public:
   Handshake() : key_{0} {}
   virtual ~Handshake() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   uint16_t get_id() const { return NET_PACKET_HANDSHAKE; };
   virtual uint32_t size() const;
   void set_key(const std::string &str) {
     pf_basic::string::safecopy(key_, str.c_str(), sizeof(key_) - 1);
   }
   const char *get_key() {
     return key_;
   }

 private:
   char key_[NET_PACKET_HANDSHAKE_KEY_SIZE];

};

class HandshakeFactory : public pf_net::packet::Factory {

 public:
   HandshakeFactory() {}
   virtual ~HandshakeFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new Handshake();
   }
   uint16_t packet_id() const {
     return NET_PACKET_HANDSHAKE;
   }
   virtual uint32_t packet_max_size() const {
     return NET_PACKET_HANDSHAKE_KEY_SIZE;
   };

};

} //namespace packet

} //namespace pf_cache

#endif //PF_NET_PACKET_HANDSHAKE_H_
