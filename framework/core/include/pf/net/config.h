/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/06/26 12:28
 * @uses The net 
*/
#ifndef PF_NET_CONFIG_H_
#define PF_NET_CONFIG_H_

#include "pf/basic/config.h"

#define NET_ONESTEP_ACCEPT_DEFAULT 50 //每帧接受新连接的默认值
#define NET_MANAGER_FRAME 100         //网络帧率
#define NET_MANAGER_CACHE_SIZE 1024   //网络管理器默认缓存大小
#define NET_PACKET_FACTORYMANAGER_ALLOCMAX (1024 * 100)
#define NET_MODULENAME "net" 

#endif //PF_NET_CONFIG_H_
