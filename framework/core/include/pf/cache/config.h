/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id config.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2016/10/08 15:20
 * @uses cache config class
 */
#ifndef PF_CACHE_CONFIG_H_
#define PF_CACHE_CONFIG_H_

#include "pf/basic/config.h"
#include "pf/sys/memory/share.h"

#define CACHE_SHARE_DEFAULT_MINUTES (10)            //默认缓存的分钟数
#define CACHE_MODULENAME "cache"
#define CACHE_WORKERS_DEFAULT (4)                   //Default workers.

namespace pf_cache {

class StoreInterface;
class Repository;
class DBStore;
class Manager;

}

/* Set cache from get value(void *) */
#define cache_set(c,v,n) { memset((c), 0, (n)); \
  auto t = reinterpret_cast<const char *>((v)); \
  memcpy((c), t, strlen(t)); \
}

#endif //PF_CACHE_CONFIG_H_
