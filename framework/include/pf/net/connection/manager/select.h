/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id select.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2014/07/17 10:56
 * @uses connection manager with select mode
 */
#ifndef PF_NET_CONNECTION_MANAGER_SELECT_H_
#define PF_NET_CONNECTION_MANAGER_SELECT_H_

#if !(OS_UNIX && defined(PF_OPEN_EPOLL)) && \
  !(OS_WIN && defined(PF_OPEN_IOCP))
#include "pf/net/connection/manager/config.h"
#include "pf/net/connection/manager/interface.h"
#include "pf/net/protocol/interface.h"

namespace pf_net {

namespace connection {

namespace manager {

class PF_API Select : public Interface {

 public:
   Select();
   virtual ~Select();

 public:
   virtual bool init(uint16_t connectionmax = NET_CONNECTION_MAX);
   virtual bool select(); //网络侦测
   virtual bool process_input(); //数据接收接口
   virtual bool process_output(); //数据发送接口
   virtual bool process_exception(); //异常连接处理
   virtual bool process_command(); //消息执行
   virtual bool heartbeat(uint32_t time = 0);

 public:
   //增加连接socket
   virtual bool socket_add(int32_t socketid, int16_t connectionid);
   //将拥有fd句柄的玩家(服务器)数据从当前系统中清除
   virtual bool socket_remove(int32_t socketid);

 private:
  //网络相关数据
   enum {
     kSelectFull = 0, //当前系统中拥有的完整句柄数据
     kSelectUse, //用于select调用的句柄数据
     kSelectMax,
   };
   fd_set readfds_[kSelectMax];
   fd_set writefds_[kSelectMax];
   fd_set exceptfds_[kSelectMax];
   timeval timeout_[kSelectMax];
   int32_t maxfd_;
   int32_t minfd_;

};

}; //namespace manager

}; //namespace connection

}; //namespace pf_net
#endif

#endif //PF_NET_CONNECTION_MANAGER_SELECT_H_
