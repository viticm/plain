/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2014/06/25 20:41
 * @uses system memory base config file
 */
#ifndef PF_SYS_MEMORY_CONFIG_H_
#define PF_SYS_MEMORY_CONFIG_H_

#include "pf/sys/config.h"

enum {
  kCmdModelClearAll = 1,
  kCmdModelLoadDump = 2,
  kCmdModelRecover = 3,
}; //Command model.

#define SYS_MEMORY_SHARENODE_DETECT_IDLE 5000
#define SYS_MEMORY_SHARENODE_SAVEINTERVAL 300000
#define SYS_MEMORY_SHARENODE_SAVECOUNT_PERTICK 5

namespace pf_sys {

namespace memory {

namespace share {

enum {
  kSmptDefault = 0,       //默认
  kSmptCache,             //缓存
  kSmptMax,               //最大
};

enum {
  kFlagFree = -1,
  kFlagSelfRead = 0x01,   //共享内存自己读取
  kFlagSelfWrite = 0x02,  //共享内存自己写
  kFlagMixedRead = 0x03,  //混合内存读取
  kFlagMixedWrite = 0x04, //混合内存写
  kFlagMax = 0x04,        //内部标记最大值
};

enum {
  kUseFree = 0,
  kUseReadyFree = 1,
  kUseFreed = 2,
  kUseHoldData = 3,
  kUseFreeEx = 4,
}; // 共享内存的使用状态

class MapPool;
class Map;

} //namespace share

} //namespace memory

} //namespace pf_sys

#endif //PF_SYS_MEMORY_CONFIG_H_
