/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id connector.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/08/07 22:27
 * @uses The connection for connector manager complement.
*/
#ifndef PF_NET_CONNECTION_MANAGER_CONNECTOR_H_
#define PF_NET_CONNECTION_MANAGER_CONNECTOR_H_

#include "pf/net/connection/manager/config.h"
#include "pf/net/connection/manager/basic.h"
#include "pf/net/socket/listener.h"

namespace pf_net {

namespace connection {

namespace manager {

class PF_API Connector : public Basic {

 public:
   Connector() {};
   virtual ~Connector() {};

 public:
   bool init(uint16_t max_size = NET_CONNECTION_MAX);
   virtual connection::Basic *connect(const char *ip, uint16_t port);
   virtual connection::Basic *group_connect(const char *ip, uint16_t port);

};

}; //namespace manager

}; //namespace connection

}; //namespace pf_net

#endif //PF_NET_CONNECTION_MANAGER_CONNECTOR_H_
