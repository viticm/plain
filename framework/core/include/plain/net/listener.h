/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id listener.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/07 21:20
 * @uses The listener for net implemention.
 */

#ifndef PLAIN_NET_LISTENER_H_
#define PLAIN_NET_LISTENER_H_

#include "plain/net/config.h"

namespace plain::net {

class PLAIN_API Listener {

 public:
  Listener();
  ~Listener();

 public:
  void start();
  void stop();

};

} // namespace plain::net

#endif // PLAIN_NET_LISTENER_H_
