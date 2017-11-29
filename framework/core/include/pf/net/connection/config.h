/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/06/26 12:31
 * @uses The net connection module config.
*/
#ifndef PF_NET_CONNECTION_CONFIG_H_
#define PF_NET_CONNECTION_CONFIG_H_

#include "pf/net/config.h"

#define NET_CONNECTION_EXECUTE_COUNT_PRE_TICK_DEFAULT 12
#define NET_CONNECTION_MAX 1024
#define NET_CONNECTION_CACHESIZE_MAX 1024
#define NET_CONNECTION_KICKTIME 6000000 //超过该时间则断开连接
#define NET_CONNECTION_INCOME_KICKTIME 60000
#define NET_CONNECTION_POOL_SIZE_DEFAULT 1280 //连接池默认大小

namespace pf_net {

namespace connection {

//压缩模式 启用压缩在性能上一定会存在部分消耗，如果流量不大建议不使用压缩模式
typedef enum {
  kCompressModeNone = 0,    //不压缩
  kCompressModeInput = 1,   //输入流压缩
  kCompressModeOutput = 2,  //输出流压缩
  kCompressModeAll = 3,     //无论是输入流还是输出流都压缩
} compress_mode_t;

class Basic;
class Pool;

}; //namespace connection

}; //namespace pf_net

#endif //PF_NET_CONNECTION_CONFIG_H_
