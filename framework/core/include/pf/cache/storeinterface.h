/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id storeinterface.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2015/04/10 15:16
 * @uses cache store base interface class
 *       cn: 这部分内容参考了PHP框架laravel的设计模式，
 *           PF2会全部统一为该模式，只实现了共享内存缓存，key为"共享内存key_缓存唯一键"
 */
#ifndef PF_CACHE_STOREINTERFACE_H_
#define PF_CACHE_STOREINTERFACE_H_

#include "pf/cache/config.h"

namespace pf_cache {

class PF_API StoreInterface {

 public:
   StoreInterface() {};
   virtual ~StoreInterface() {};

 public:
   
   //Retrieve an item from the cache by key.
   virtual void *get(const char *key) = 0;
   
   //Store an item in the cache for a given number of minutes.
   virtual void put(const char *key, void *value, int32_t minutes) = 0;
   
   //Store an item in the cache indefinitely.
   virtual void forever(const char *key, void *value) {
     put(key, value, -1);
   };

   //Change an item hook time.
   virtual void hook(const char *key, int32_t minutes) = 0;
   
   //Remove an item from the cache.
   virtual void forget(const char *key) = 0;

   //Remove all items from the cache.
   virtual void flush() = 0;

   //Get the cache key prefix.
   virtual const char *get_prefix() const = 0;

   //Tick for logic.
   virtual void tick() = 0;

   //Set an item cache time.
   virtual void set_cache_time(const char *key, int32_t minutes) = 0;

   //Set cache count.
   virtual bool set_cache_count(int32_t count) = 0;

   //Store an item can recycle when the cache is full.
   virtual void recycle(const char *key) = 0;

};

} //namespace pf_cache

#endif //PF_CACHE_STOREINTERFACE_H_
