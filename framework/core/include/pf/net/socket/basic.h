/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id base.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/19 14:51
 * @uses socket base class
 */
#ifndef PF_NET_SOCKET_BASE_H_
#define PF_NET_SOCKET_BASE_H_

#include "pf/basic/string.h"
#include "pf/net/socket/api.h"

namespace pf_net {

namespace socket {

class PF_API Basic {
 
 public:
   Basic();
   Basic(const char *host, uint16_t port);
   virtual ~Basic();

 public: //socket base operate functions
   bool create();
   void close();
   bool connect(); //use self host_ and port_
   bool connect(const char *host, uint16_t port);
   bool reconnect(const char *host, uint16_t port);
   int32_t send(const void *buffer, uint32_t length, uint32_t flag = 0);
   int32_t receive(void *buffer, uint32_t length, uint32_t flag = 0);
   uint32_t available() const;
   int32_t accept(struct sockaddr_in *accept_sockaddr_in = nullptr);
   bool bind(const char *ip = nullptr);
   bool bind(uint16_t port, const char *ip = nullptr);
   bool listen(uint32_t backlog);
   static int32_t select(int32_t maxfdp, 
                         fd_set *readset, 
                         fd_set *writeset, 
                         fd_set *exceptset,
                         timeval *timeout);

 public: //socket check and set functions
   uint32_t get_linger() const;
   bool set_linger(uint32_t lingertime);
   bool is_reuseaddr() const;
   bool set_reuseaddr(bool on = true);
   uint32_t get_last_error_code() const;
   void get_last_error_message(char *buffer, uint16_t length) const;
   bool error() const; //socket if has error
   bool is_nonblocking() const;
   bool set_nonblocking(bool on = true);
   uint32_t get_receive_buffer_size() const;
   bool set_receive_buffer_size(uint32_t size);
   uint32_t get_send_buffer_size() const;
   bool set_send_buffer_size(uint32_t size);
   uint16_t port() const { return port_; };
   uint64_t uint64host() const;
   const char *host() { return host_; };
   bool is_valid() const { return id_ != SOCKET_INVALID; };
   int32_t get_id() const { return id_; };
   
 public:
   void set_id(int32_t id) { id_ = id; };
   void set_host(const char *_host) { 
     pf_basic::string::safecopy(host_, _host, sizeof(host_));
   };
   void set_port(uint16_t _port) { port_ = _port; };

 private:
   int32_t id_;
   char host_[IP_SIZE]; //两层含义，连接时则为目的IP，接受时为客户IP
   uint16_t port_;

};

} //namespace socket

} //namespace pf_net

#endif //PF_NET_SOCKET_BASE_H_
