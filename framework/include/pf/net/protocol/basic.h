/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id basic.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/07/25 20:16
 * @uses The protocol basic for net.
*/
#ifndef PF_NET_PROTOCOL_BASIC_H_
#define PF_NET_PROTOCOL_BASIC_H_

#include "pf/net/protocol/config.h"
#include "pf/net/protocol/interface.h"

namespace pf_net {

namespace protocol {

class PF_API Basic : public Interface {

 public:
   Basic();
   virtual ~Basic();

 public:
   //处理网络包
   virtual bool command(connection::Basic *connection, uint16_t count);
   //处理压缩数据，主要是将压缩的数据解压
   virtual bool compress(connection::Basic *connection, 
                         char *uncompress_buffer, 
                         char *compress_buffer);
   virtual bool send(connection::Basic *connection, packet::Interface *packet);
   virtual size_t header_size() const { return NET_PACKET_HEADERSIZE; };

};

}; //namespace protocol

}; //namespace pf_net

#endif //PF_NET_PROTOCOL_BASIC_H_
