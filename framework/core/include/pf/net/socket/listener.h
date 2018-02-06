/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id listener.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2016/09/15 02:18
 * @uses server net model socket class
 */
#ifndef PF_NET_SOCKET_SERVER_H_
#define PF_NET_SOCKET_SERVER_H_

#include "pf/net/socket/basic.h"

namespace pf_net {

namespace socket {

class PF_API Listener {

 public:
   Listener(uint16_t port, const std::string &ip = "", uint32_t backlog = 5);
   ~Listener();

 public:
   void close();
   bool accept(pf_net::socket::Basic *socket);
   uint32_t get_linger() const;
   bool set_linger(uint32_t lingertime);
   bool is_nonblocking() const;
   bool set_nonblocking(bool on = true);
   uint32_t get_receive_buffer_size() const;
   bool set_receive_buffer_size(uint32_t size);
   uint32_t get_send_buffer_size() const;
   bool set_send_buffer_size(uint32_t size);
   int32_t get_id() const { 
     return socket_ ? socket_->get_id() : SOCKET_INVALID; 
   };
   uint16_t port() const { return socket_ ? socket_->port() : 0; };
   const char *host() { return socket_ ? socket_->host() : ""; };

 protected:
   std::unique_ptr<pf_net::socket::Basic> socket_;

};

} //namespace socket

} //namespace pf_net

#endif //PF_NET_SOCKET_SERVER_H_
