/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/09/03 16:49
 * @uses The net rpc config file.
 */

#ifndef PLAIN_NET_RPC_CONFIG_H_
#define PLAIN_NET_RPC_CONFIG_H_

#include "plain/net/config.h"

namespace plain::net {
namespace rpc {

enum class Type : uint8_t {
  Null = 0,
  Int8 = 1,
  Uint8 = 2,
  Int16 = 3,
  Uint16 = 4,
  Int32 = 5,
  Uint32 = 6,
  Int64 = 7,
  Uint64 = 8,
  Float32 = 9,
  Float64 = 10,
  TrueBool = 11,
  FalseBool = 12,
  Bin8 = 13,
  Bin16 = 14,
  Bin32 = 15,
  Ext8 = 16,
  Ext16 = 17,
  Ext32 = 18,
  Str8 = 19,
  Str16 = 20,
  Str32 = 21,
  Array16 = 22,
  Array32 = 23,
  Map16 = 24,
  Map32 = 25,
  Max,
};

class Packer;
class Unpacker;
class Dispatcher;
class Listener;
class Connector;

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_CONFIG_H_
