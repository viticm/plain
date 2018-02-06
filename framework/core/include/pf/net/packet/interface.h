/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id base.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/20 11:54
 * @uses server and client net pakcet class
 */
#ifndef PF_NET_PACKET_BASE_H_
#define PF_NET_PACKET_BASE_H_

#include "pf/net/socket/config.h"
#include "pf/net/stream/input.h"
#include "pf/net/stream/output.h"
#include "pf/net/packet/config.h"
#include "pf/net/connection/config.h"

namespace pf_net {

namespace packet {

class PF_API Interface {

 public:
   Interface() {};
   virtual ~Interface() {};

 public:
   virtual void clear() {};
   virtual bool read(stream::Input &) = 0;
   virtual bool write(stream::Output &) = 0;
   virtual uint32_t execute(connection::Basic *connection);
   virtual uint16_t get_id() const = 0;
   virtual uint32_t size() const = 0;
   virtual void set_id(uint16_t) {};
   virtual void set_size(uint32_t) {};
   int8_t get_index() const { return index_; };
   void set_index(int8_t index) { index_ = index; };
   uint8_t get_status() const { return status_; };
   void set_status(uint8_t status) { status_ = status; };

 private:
   int8_t status_;
   int8_t index_;

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_BASE_H_
