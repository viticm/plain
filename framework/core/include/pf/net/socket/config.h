/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/06/26 12:58
 * @uses The net socket config file.
*/
#ifndef PF_NET_SOCKET_CONFIG_H_
#define PF_NET_SOCKET_CONFIG_H_

#include "pf/net/config.h"

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef SOCKET_INVALID
#define SOCKET_INVALID -1
#endif

#if OS_WIN
  
#ifndef EINPROGRESS 
#define EINPROGRESS WSAEINPROGRESS
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOC
#endif

#endif

#define SOCKET_WOULD_BLOCK EWOULDBLOCK //api use SOCKET_ERROR_WOULD_BLOCK
#define SOCKET_CONNECT_ERROR EINPROGRESS
#define SOCKET_CONNECT_TIMEOUT 10

namespace pf_net {

namespace socket {

class Basic;
class Listener;

typedef struct streamdata_struct {
  char *buffer;
  uint32_t bufferlength;
  uint32_t bufferlength_max;
  uint32_t head;
  uint32_t tail;
  streamdata_struct() :
    buffer{nullptr},
    bufferlength{0},
    bufferlength_max{0},
    head{0},
    tail{0} {
  }
} streamdata_t;

}; //namespace socket

}; //namespace pf_net

#endif //PF_NET_SOCKET_CONFIG_H_
