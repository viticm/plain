/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/07/03 22:17
 * @uses The net module packet config.
 *       如果外部自主控制协议，网络包可以设置为自主控制，不再走统一的接口。
 *       现在的网络包分为一般模式，还有就是走外部协议模式，如何兼容？
 *       在这里需要向一下网络包与协议的区分。
*/
#ifndef PF_NET_PACKET_CONFIG_H_
#define PF_NET_PACKET_CONFIG_H_

#include "pf/net/config.h"

#define NET_PACKET_DYNAMIC_ONCESIZE (1024) //动态网络包每次重新增加的内存大小
#define NET_PACKET_DYNAMIC_SIZEMAX (1024 * 100) //动态网络包的最大内存大小 

#define NET_PACKET_GETINDEX(a) ((a) >> 24)
#define NET_PACKET_SETINDEX(a,index) ((a) = (((a) & 0xffffff) + ((index) << 24)))
#define NET_PACKET_GETLENGTH(a) ((a) & 0xffffff)
#define NET_PACKET_SETLENGTH(a,length) ((a) = ((a) & 0xff000000) + (length))
//note cn（默认规则）:
//消息头中包括：uint16_t - 2字节；uint32_t - 4字节中高位一个字节为消息序列号，
//其余三个字节为消息长度
//通过GET_PACKETINDEX和GET_PACKETLENGTH宏，
//可以取得UINT数据里面的消息序列号和长度
//通过SET_PACKETINDEX和SET_PACKETLENGTH宏，
//可以设置UINT数据里面的消息序列号和长度
#define NET_PACKET_HEADERSIZE (sizeof(uint16_t) + sizeof(uint32_t))
#define NET_PACKET_HEADERSIZE_MAX 16 //头长度的最大值，实际由
                                     //protocol::header_size控制长度
                                     //因为这里有可能存在没有消息头的情况

typedef enum {
  kPacketExecuteStatusError = 0,  //表示出现严重错误，当前连接需要被强制断开 
  kPacketExecuteStatusBreak,      //表示返回后剩下的消息将不在当前处理循环里处理
  kPacketExecuteStatusContinue,   //表示继续在当前循环里执行剩下的消息
  kPacketExecuteStatusNotRemove,  //表示继续在当前循环里执行剩下的消息,
                                  //但是不回收当前消息
  kPacketExecuteStatusNotRemoveError,
} packet_executestatus_t;

#define NET_PACKET_HANDSHAKE 0xfff0 //The safe encrypt packet id(65520).
#define NET_PACKET_HANDSHAKE_KEY_SIZE (128) //The safe encrypt key size;
#define NET_PACKET_ID_NORMAL_BEGIN (0x0001) //The default normal packet id begin.
#define NET_PACKET_ID_NORMAL_END (0x4e20) //The default normal packt id end(20000).
#define NET_PACKET_ID_DYNAMIC_BEGIN (0x04e21) // The default dynamic packet id begin(20001).
#define NET_PACKET_ID_DYNAMIC_END (0xffef) // The default dynamic packet id end(65519).

typedef enum {
  kPacketFlagNone = 0,
  kPacketFlagRemove,
} packetflag_t;


namespace pf_net {

namespace packet {

class Interface;
class Dynamic;
class Factory;
class FactoryManager;

typedef PF_API struct queue_struct queue_t;

struct queue_struct {
  Interface *packet;
  uint16_t connectionid;
  uint32_t flag;
  queue_struct() :
    packet{nullptr},
    connectionid{static_cast<uint16_t>(ID_INVALID)},
    flag{kPacketFlagNone} {
  };
  ~queue_struct();
};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_CONFIG_H_
