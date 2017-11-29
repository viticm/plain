/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id repository.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2015/04/10 15:27
 * @uses cache repository class
 */
#ifndef PF_CACHE_REPOSITORY_H_
#define PF_CACHE_REPOSITORY_H_

#include "pf/cache/config.h"
#include "pf/basic/hashmap/template.h"

typedef void *(__stdcall *function_callback)();

namespace pf_cache {

class PF_API Repository {

 public:
   Repository(StoreInterface *store);
   ~Repository();

 public:
   
   //Determine if an item exists in the cache.
   bool has(const char *key);
   
   //Retrieve an item from the cache by key.
   void *get(const char *key, void *_default = nullptr);

   //Retrieve an item from the cache and delete it.
   void *pull(const char *key, void *_default = nullptr);

   //Store an item in the cache.
   void put(const char *key, 
            void *value, 
            int32_t minutes = CACHE_SHARE_DEFAULT_MINUTES);

   //Store an item in the cache if the key does not exist.
   void add(const char *key, 
            void *value, 
            int32_t minutes = CACHE_SHARE_DEFAULT_MINUTES);

   //Get an item from the cache, or store the default value.
   void *remember(const char *key, int32_t minutes, function_callback callback);

   //Get an item from the cache, or store the default value forever.
   void *sear(const char *key, function_callback callback);

   //Get an item from the cache, or store the default value forever.
   void *remember_forever(const char *key, function_callback callback);

   void forget(const char *key);

   //Get the default cache time.
   int32_t get_default_cachetime() const;

   //Hook catch to long time.
   void hook(const char *key, int32_t minutes = CACHE_SHARE_DEFAULT_MINUTES);

   //Set the default cache time in minutes.
   void set_default_cachetime(int32_t minutes);

   //Get the cache store implementation.
   StoreInterface *store() { return store_; };

   //Set an item cache time.
   void set_cache_time(const char *key, int32_t minutes);
   
   //Tick for logic.
   void tick();

   //Set cache count.
   bool set_cache_count(int32_t count);

   //Store an item can recycle when the cache is full.
   void recycle(const char *key);

 protected:

   //The cache store implementation.
   StoreInterface *store_;

   //The default number of minutes to store items.
   int32_t minutes_;

};

}; //namespace pf_cache

#endif //PF_CACHE_REPOSITORY_H_
