/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/28 11:28
 * @uses The net config file.
 *       Mode Iocp use https://github.com/piscisaureus/wepoll to implemention.
 */

#ifndef PLAIN_NET_CONFIG_H_
#define PLAIN_NET_CONFIG_H_

#include "plain/basic/config.h"

namespace plain::net {

enum class Mode {
  Select = 0,
  Epoll = 1,
  IoUring = 2,
  Iocp = 3,
  Kqueue = 4,
};

class Address;
class Connector;
class Listener;
class Protocol;

namespace socket {

class Basic;
class Listener;

}

namespace connection {

class Basic;

class Manager;
class Select;
class Epoll;
class IoUring;
class Iocp;

} // namespace connection

namespace stream {

class Codec;
class Basic;

} // namespace stream

namespace packet {

class Basic;

struct limit_struct {
  uint32_t max_id{std::numeric_limits<uint16_t>::max()};
  uint32_t max_length{200 * 1024}; // default 200k
};
using limit_t = limit_struct;

} // namespace packet

struct setting_struct {
  setting_struct() = default;
  ~setting_struct() = default;
  uint32_t max_count{1024};
  uint32_t default_count{32};
  Mode mode{Mode::Select};
  // ip_v4: x.x.x.x:port ip_v6: [x:x:x:...]:port
  std::string address; // listener only
  std::string name;
  packet::limit_t packet_limit;
};

using setting_t = setting_struct;

} // namespace plain::net

#endif // PLAIN_NET_CONFIG_H_
