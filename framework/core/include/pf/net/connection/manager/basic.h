/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id basic.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/07/10 19:04
 * @uses The baisc manager for net.
*/
#ifndef PF_NET_CONNECTION_MANAGER_BASIC_H_
#define PF_NET_CONNECTION_MANAGER_BASIC_H_

#include "pf/net/connection/manager/config.h"
#include "pf/net/connection/manager/epoll.h"
#include "pf/net/connection/manager/select.h"

namespace pf_net {

namespace connection {

namespace manager {

#if OS_UNIX && defined(PF_OPEN_EPOLL) /* { */  
class PF_API Basic : public Epoll {
#elif OS_WIN && defined(PF_OPEN_IOCP) /* }{ */
class PF_API Basic : public Iocp {
#else /* }{ */
class PF_API Basic : public Select {
#endif /* } */
 public:
   Basic() {};
   virtual ~Basic() {};

 public:
   virtual bool heartbeat(uint64_t time = 0);
   virtual void tick();

};

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif //PF_NET_CONNECTION_MANAGER_BASIC_H_
