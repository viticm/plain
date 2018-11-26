/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id listener.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/07/10 18:10
 * @uses The connection for listener manager complement.
*/
#ifndef PF_NET_CONNECTION_MANAGER_LISTENER_H_
#define PF_NET_CONNECTION_MANAGER_LISTENER_H_

#include "pf/net/connection/manager/config.h"
#include "pf/net/connection/manager/basic.h"
#include "pf/net/socket/listener.h"

namespace pf_net {

namespace connection {

namespace manager {

class PF_API Listener : public Basic {

 public:
   Listener();
   virtual ~Listener();

 public:
   virtual bool is_service() const { return true; }

 public:
   bool init(uint16_t max_size, uint16_t port, const std::string &ip);
   uint16_t port() const { 
     return listener_socket_ ? listener_socket_->port() : 0; 
   };
   const char *host() {
     return listener_socket_ ? listener_socket_->host() : "";
   }
   virtual connection::Basic *accept(); //新连接接受处理

 public:

   virtual void on_connect(connection::Basic * connection);

 public:

   int32_t listener_socket_id() const {
     return listener_socket_ ? listener_socket_->get_id() : SOCKET_INVALID;
   }
   //If set safe encrypt string then all connection will check it. 
   void set_safe_encrypt_str(const std::string &str) {
     safe_encrypt_str_ = str;
   }
   const std::string get_safe_encrypt_str() {
     return safe_encrypt_str_;
   }

 private:
   std::unique_ptr<socket::Listener> listener_socket_;
   std::string safe_encrypt_str_;
   bool ready_;

};

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif //PF_NET_CONNECTION_MANAGER_LISTENER_H_
