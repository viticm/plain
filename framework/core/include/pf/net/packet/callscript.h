/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id callscript.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/21 10:41
 * @uses The execute script function with net.
*/
#ifndef PF_NET_PACKET_CALLSCRIPT_H_
#define PF_NET_PACKET_CALLSCRIPT_H_

#include "pf/basic/string.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace packet {

class PF_API CallScript : public pf_net::packet::Interface {

 public:
   CallScript() : func_{0}, params_{""}, eid_{-1} {}
   virtual ~CallScript() {}

 public:
   virtual bool read(pf_net::stream::Input &);
   virtual bool write(pf_net::stream::Output &);
   virtual uint32_t execute(pf_net::connection::Basic *connection);
   uint16_t get_id() const { return NET_PACKET_CALLSCRIPT; };
   virtual uint32_t size() const;
   void set_func(const std::string &str) {
     pf_basic::string::safecopy(func_, str.c_str(), sizeof(func_) - 1);
   }
   const char *get_func() {
     return func_;
   }
   void set_params(const std::vector<std::string> &params);
   void get_params(std::vector<std::string> &params);

 private:
   char func_[128];
   std::string params_;
   int8_t eid_;

};

class CallScriptFactory : public pf_net::packet::Factory {

 public:
   CallScriptFactory() {}
   virtual ~CallScriptFactory() {}

 public:
   virtual pf_net::packet::Interface *packet_create() {
     return new CallScript();
   }
   uint16_t packet_id() const {
     return NET_PACKET_CALLSCRIPT;
   }
   virtual uint32_t packet_max_size() const {
     return 100 * 1024;
   };

};

} //namespace packet

} //namespace pf_cache

#endif //PF_NET_PACKET_CALLSCRIPT_H_
