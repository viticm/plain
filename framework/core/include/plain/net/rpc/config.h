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
  // positive fixint = 0x00 - 0x7f
  // fixmap = 0x80 - 0x8f
  // fixarray = 0x90 - 0x9a
  // fixstr = 0xa0 - 0xbf
  // negative fixint = 0xe0 - 0xff

  /*
  Null = 0xc0,
  FalseBool = 0xc2,
  TrueBool = 0xc3,
  Bin8 = 0xc4,
  Bin16 = 0xc5,
  Bin32 = 0xc6,
  Ext8 = 0xc7,
  Ext16 = 0xc8,
  Ext32 = 0xc9,
  Float32 = 0xca,
  Float64 = 0xcb,
  Uint8 = 0xcc,
  Uint16 = 0xcd,
  Uint32 = 0xce,
  Uint64 = 0xcf,
  Int8 = 0xd0,
  Int16 = 0xd1,
  Int32 = 0xd2,
  Int64 = 0xd3,
  Fixext1 = 0xd4,
  Fixext2 = 0xd5,
  Fixext4 = 0xd6,
  Fixext8 = 0xd7,
  Fixext16 = 0xd8,
  Str8 = 0xd9,
  Str16 = 0xda,
  Str32 = 0xdb,
  Array16 = 0xdc,
  Array32 = 0xdd,
  Map16 = 0xde,
  Map32 = 0xdf
  */

};

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_CONFIG_H_
