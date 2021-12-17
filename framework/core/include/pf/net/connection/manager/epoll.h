/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id epoll.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2014/07/17 13:40
 * @uses connection manager with select mode
 */
#ifndef PF_NET_CONNECTION_MANAGER_EPOLL_H_
#define PF_NET_CONNECTION_MANAGER_EPOLL_H_

#if OS_UNIX && defined(PF_OPEN_EPOLL)
#include "pf/net/connection/manager/interface.h"
#include "pf/net/socket/extend.inl"
#include "pf/net/protocol/interface.h"

namespace pf_net {

namespace connection {

namespace manager {

class PF_API Epoll : public Interface {

 public:
   Epoll();
   virtual ~Epoll();

 public:
   virtual bool init(uint16_t connectionmax = NET_CONNECTION_MAX);
   virtual bool select();             //网络侦测
   virtual bool process_input();      //数据接收接口
   virtual bool process_output();     //数据发送接口
   virtual bool process_exception();  //异常连接处理
   virtual bool process_command(); //消息执行
   virtual bool heartbeat(uint64_t time = 0);

 public:
   virtual bool socket_add(int32_t socketid, int16_t connectionid);
   //将拥有fd句柄的玩家(服务器)数据从当前系统中清除
   virtual bool socket_remove(int32_t socketid);

 public:
   bool poll_set_max_size(uint16_t max_size);

 private:
   polldata_t polldata_;

};

} //namespace manager

} //namespace connection

} //namespace pf_net


#endif

#endif //PF_NET_CONNECTION_MANAGER_EPOLL_H_
