/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id dbstore.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2015/04/10 16:11
 * @uses db cache store class
 *       cn: 数据库缓存，可实现定时保存的功能，目前该功能的实现为共享内存结构
 *           共享内存key如果未被注册，则自动注册为默认大小
 *           所有的key在这里必须存储为tablename#only_key
 *           即key的存储方式必须为表名#唯一key.
 *          
 *       2016/10/28 修复缓存key以及回收缓存的问题(采用sharemap)
 *       2016/12/30 考虑DB类的保存使用统一的接口，对查询中和需要保存的做一份临时
 *                  HASH，不再使用永久HASH来对保存、查询进行操作
 *       2017/01/05 考虑将查询的权利完全交给使用者处理，服务不自动管理，需要考虑
 *                  脏数据的移除方法（脏数据是指该缓存已经没有意义，外部已经
 *                  不再拥有的数据）
 *                  目前的脏数据可能会由外部关联进程宕机、自身进程宕机或者外部没
 *                  有自动管理缓存状态
 *                  脏数据的回收处理：
 *                  1、外部应用调用统一接口更新拥有缓存的访问时间
 *                  2、当前时间 - 缓存的访问时间 = 空闲缓存时间
 *                  3、当空闲缓存时间超过一定时间则触发脏数据回收机制
 *                  4、脏数据可能需要被保存
 *                  5、脏数据的定时循环检测可能在十分钟左右执行（
 *                  尽量在进程空闲期间）
 *       2017/01/06 对缓存有两个区别：服务者和使用者，服务者负责内存的创建、销毁
 *                  等，使用者负责内存中数据的改变，内存中数据发生改变时可能触发
 *                  服务者的一些行为（在同一进程中，服务者与使用者可以共存）
 *       2017/03/31 将考虑使用lua栈以及gc方式来优化该部分的代码（share memory）
 *       2017/04/17 lua栈的实现方式放缓，考虑以当前模式下的效率是否能够满足，
 *                  之后再优化
 *                  
 */
#ifndef PF_CACHE_DB_STORE_H_
#define PF_CACHE_DB_STORE_H_

#include "pf/cache/config.h"
#include "pf/db/config.h"
#include "pf/net/connection/config.h"
#include "pf/net/connection/manager/config.h"
#include "pf/sys/memory/sharemap.h"
#include "pf/sys/memory/share.h"
#include "pf/sys/config.h"
#include "pf/cache/storeinterface.h"
#include "pf/cache/db_define.h"

namespace pf_cache {

class SharePool : public pf_sys::memory::share::GroupPool {

 public:
   explicit SharePool(
       uint32_t _key, 
       const std::vector<pf_sys::memory::share::group_item_t> &group) :
     pf_sys::memory::share::GroupPool(_key, group) {

  }

   ~SharePool() {}

 public:
   
   //Get the base table data.
   inline db_table_base_t *table_base(int16_t index, int16_t data_index) {
     char *data = item_data_header(index, data_index);
     return reinterpret_cast< db_table_base_t * >(data);
   };
   
   //Get the columus and types of table.
   inline db_table_info_t *table_info(int16_t index, int16_t data_index) {
     char *data = item_data_header(index, data_index);
     size_t exculde = sizeof(db_table_base_t);
     return reinterpret_cast< db_table_info_t * >(data + exculde);
   };

   //Get the item data.
   inline db_item_t *item(int16_t index, int16_t data_index) {
     char *data = item_data(index, data_index);
     return reinterpret_cast< db_item_t * >(data);
   };

   //Get real data.
   inline char *real_data(int16_t index, int16_t data_index) {
     char *data = item_data(index, data_index);
     return data + sizeof(db_item_t);
   }

};

class PF_API DBStore : public StoreInterface {

 public:
   DBStore();
   virtual ~DBStore();

 public:
   typedef struct cache_info_struct {
     int32_t share_key;
     int32_t share_index;
     int32_t recycle_index;
     std::string name;
     std::string only_key;
     cache_info_struct() : 
      share_key{ID_INVALID},
      share_index{INDEX_INVALID},
      recycle_index{INDEX_INVALID},
      name{""},
      only_key{""} {

     }
   } cache_info_t;

 public:

   /* All key is tablename#key */

   //Retrieve an item from the cache by key.
   virtual void *get(const char *key);

   //Store an item in the cache for a given number of minutes.
   virtual void put(const char *key, void *value, int32_t minutes);

   //Remove an item from the cache.
   virtual void forget(const char *key);

   //Remove all items from the cache.
   virtual void flush();

   //Get the cache key prefix.
   virtual const char *get_prefix() const { return ""; };

   //Set an item cache time.
   virtual void set_cache_time(const char *key, int32_t minutes) {
     hook(key, minutes);
   };
   
   //Tick for logic.
   virtual void tick();

   //Get cache prefix;
   virtual const char *get_prefix() { return nullptr; };

   //Hook one cache.
   virtual void hook(const char *key, int32_t minutes) {
     auto cache = getitem(key);
     if (!cache) return;
     cache->hook_time = minutes * 60;
   };

   //Store an item can recycle when the cache is full.
   inline void recycle(const char *key) {
     cache_info_t info;
     cache_info(key, info);
     recycle_push(info.share_key, info.only_key);
   }

   //Set cache count.
   virtual bool set_cache_count(int32_t) { return true; };
  
   //Set the share service.
   void set_service(bool flag) { service_ = flag; };

   //Query sql from db.
   bool query(const std::string &key);

   //Wait query in query list.
   bool waitquery(const char *key);

   //Get the fetch array from cache.
   bool get(const std::string &key, db_fetch_array_t &hash);

   //Set cache from fetch array.
   bool set(const std::string &key, const db_fetch_array_t &hash);

   //Generate sql string of cache data.
   bool generate_sql(const std::string &key, std::string &sql);

   //Get the item value from cache.
   db_item_t *getitem(const std::string &key);

   //Forget all from the only key.
   void forgetall(const std::string &only_key);
 
 public:

   //Get the db conection.
   using get_db_connection_func = 
     std::function< pf_net::connection::Basic * (db_item_t &) >;

 public:

   //Set query by net.
   void set_query(pf_net::connection::manager::Basic *net_manager, 
                  get_db_connection_func func,
                  uint16_t query_id = CACHE_SHARE_NET_QUERY_PACKET_ID,
                  uint16_t result_id = CACHE_SHARE_NET_RESULT_PACKET_ID) {
     net_manager_ = net_manager;
     get_db_connection_func_ = func;
     query_net_ = true;
     packet_id_.query = query_id;
     packet_id_.result = result_id;
   };

   //Set query by directly(deafult).
   void set_query(pf_db::Interface *db_env) {
     db_env_ = db_env;
     query_net_ = false;
   };

   //Set db type.
   void set_dbtype(dbenv_t dbenv) {
     dbenv_ = dbenv;
   }

 public: //For sharememory.

   void set_key(int32_t key_map, int32_t recycle_map, int32_t query_map) {
     keys_.key_map = key_map;
     keys_.recycle_map = recycle_map;
     keys_.query_map = query_map;
   };

   bool load_config(const std::string &file_name);

   bool init();

   //Get the cache by strings.
   bool get(const std::string &key, char *&columns, char * &rows);

   //Set the cache by strings.
   bool set(const std::string &key, 
            const std::string &columns, 
            const std::string &rows);

   //Free the recycle as you want size(0 mean free full as possible)
   size_t recycle_free(int32_t key, size_t size = 0);

   pf_sys::memory::share::Map *get_keymap() {
     return &key_map_;
   }

   size_t get_query_size() const {
     return query_map_.size();
   }

   size_t get_forget_size() const {
     return forgetlist_.size();
   }

 private:

   //Cache key is valid.
   bool cache_key_is_valid(const char *key);

   //Cache key info.
   void cache_info(const char *key, cache_info_t &cache_info);

   //Check the fetch array is valid for cache.
   bool hash_is_valid(const db_fetch_array_t &hash, size_t size);
  
 private:

   //Get can remove cache only key from recycle.
   bool recycle_pop(int32_t key, std::string &only_key);

   //Cache alive by key and only key without the name.
   bool recycle_push(int32_t key, const std::string &only_key);

   //Remove all cache from recycle index.
   void recycle_remove(int32_t key, 
                       const std::string &only_key, 
                       const std::string &except_key = "");

   //Drop a cache from recycle index.
   void recycle_drop(int32_t key, int32_t index);

   //Change all cache from recycle to the new index.
   void recycle_mod(int32_t key, const std::string &only_key, int32_t index);

   //Check the cache if in recycle.
   bool recycle_find(int32_t key, int32_t index);

 private:
   
   //The key index hash map, for share memory 
   //["name#key"] = index#recycle_index.
   //recycle_index is -1 then not in recycle.
   //Remember the hash map base types.
   pf_sys::memory::share::Map key_map_;

   //The recycle keys.
   //["count_key"] is the recycle count of this key.
   pf_sys::memory::share::Map recycle_map_;

   //The query keys.
   pf_sys::memory::share::Map query_map_;

   //Share keys.
   struct {
     int32_t key_map;
     int32_t recycle_map;
     int32_t query_map;
   } keys_;

   //Share packet ID.
   struct {
     uint16_t query;
     uint16_t result;
   } packet_id_;

   //The service is true then will try create share memory.
   bool service_;

   //The share memory hash.
   std::map< int32_t, std::unique_ptr< SharePool > > share_pool_map_;

   //The share memory config hash([tablename] = config).
   pf_basic::hashmap::Template< std::string, db_share_config_t > 
     share_config_map_;

   //The share group config hash([sharekey] = config).
   std::map< int32_t, std::vector< pf_sys::memory::share::group_item_t > >
     share_group_map_;

   //The recycle size map, to key_map_.
   pf_basic::hashmap::Template< int32_t, int32_t > recycle_size_map_;

   //The hash size map, to recycle_map_.
   pf_basic::hashmap::Template< int32_t, int32_t > hash_size_map_;

   using size_map_iterator = 
     pf_basic::hashmap::Template< int32_t, int32_t >::iterator_t;

   using group_map_iterator = std::map< 
     int32_t, std::vector< pf_sys::memory::share::group_item_t > >::iterator;

   using share_config_iterator = 
     pf_basic::hashmap::Template< std::string, db_share_config_t >::iterator_t;

   using share_pool_iterator = 
     std::map< int32_t, std::unique_ptr< SharePool > >::iterator;

   //Ready flag.
   bool ready_;

   //Query by net.
   bool query_net_;

   //The query db manager;
   pf_db::Interface *db_env_;

   //The net manager.
   pf_net::connection::manager::Basic *net_manager_;

   //The get net connection function.
   get_db_connection_func get_db_connection_func_;

   //The workers for tick.
   std::unique_ptr<pf_sys::ThreadPool> workers_;

   //The cache last check time.
   uint32_t cache_last_check_time_;

   //The forget list keys.
   std::vector<std::string> forgetlist_;

   //The db type for cache.
   dbenv_t dbenv_;

};

} //namespace pf_cache

#endif //PF_CACHE_DB_STORE_H_
