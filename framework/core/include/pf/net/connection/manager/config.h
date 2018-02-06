/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/06/26 12:33
 * @uses The net connection manager config.
*/
#ifndef PF_NET_CONNECTION_MANAGER_CONFIG_H_
#define PF_NET_CONNECTION_MANAGER_CONFIG_H_

#include "pf/net/connection/config.h"
#include "pf/net/packet/config.h"

namespace pf_net {

namespace connection {

namespace manager {

class Basic;
class Listener;
class Connector;
class Epool;
class Iocp;
class Select;

typedef PF_API struct cache_struct cache_t;
struct cache_struct {
  packet::queue_t *queue;
  uint32_t head;
  uint32_t tail;
  uint32_t size;
  cache_struct() : 
    queue{nullptr},
    head{0},
    tail{0},
    size{0}
  {};
  ~cache_struct() {
    safe_delete_array(queue);
  };
};

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif //PF_NET_CONNECTION_MANAGER_CONFIG_H_
