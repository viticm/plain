/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id register_connection_name.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/10 11:03
 * @uses Register the current connection each name.
*/
#ifndef PF_NET_PACKET_REGISTER_CONNECTION_NAME_H_
#define PF_NET_PACKET_REGISTER_CONNECTION_NAME_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class RegisterConnectionName : public pf_net::packet::Interface {

 public:
   RegisterConnectionName() : name_{""} {}
   virtual ~RegisterConnectionName() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   virtual uint32_t size() const;
   uint16_t get_id() const { return NET_PACKET_REGISTER_CONNECTION_NAME; };
   void set_name(const std::string &name) {
     pf_basic::string::safecopy(name_, name.c_str(), sizeof(name_) - 1);
   };

 private:
   char name_[128];

};

class RegisterConnectionNameFactory : public pf_net::packet::Factory {

 public:
   RegisterConnectionNameFactory() {}
   virtual ~RegisterConnectionNameFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new RegisterConnectionName();
   }
   uint16_t packet_id() const {
     return NET_PACKET_REGISTER_CONNECTION_NAME;
   }
   virtual uint32_t packet_max_size() const {
     return 128;
   };

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_REGISTER_CONNECTION_NAME_H_
