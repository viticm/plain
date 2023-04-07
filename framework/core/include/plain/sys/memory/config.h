/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/06 21:39
 * @uses system memory base config file
 */
#ifndef PLAIN_SYS_MEMORY_CONFIG_H_
#define PLAIN_SYS_MEMORY_CONFIG_H_

#include "plain/sys/config.h"

#define SYS_MEMORY_SHARENODE_DETECT_IDLE 5000
#define SYS_MEMORY_SHARENODE_SAVEINTERVAL 300000
#define SYS_MEMORY_SHARENODE_SAVECOUNT_PERTICK 5

namespace plain {

enum class CmdMode {
  ClearAll = 1,
  LoadDump = 2,
  Recover = 3,
}; //Command model.

namespace memory {

namespace share {

enum class Smpt {
  Default = 0,       //默认
  Cache,             //缓存
  Max,               //最大
};

enum class Flag {
  Free = -1,
  SelfRead = 0x01,   //共享内存自己读取
  SelfWrite = 0x02,  //共享内存自己写
  MixedRead = 0x03,  //混合内存读取
  MixedWrite = 0x04, //混合内存写
  Max = 0x04,        //内部标记最大值
};

enum class Use {
  Free = 0,
  ReadyFree = 1,
  Freed = 2,
  HoldData = 3,
  FreeEx = 4,
}; // 共享内存的使用状态

class MapPool;
class Map;

} //namespace share

} //namespace memory

} //namespace plain

#endif //PLAIN_SYS_MEMORY_CONFIG_H_
