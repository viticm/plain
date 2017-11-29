#include "pf/file/tab.h"
#include "pf/basic/logger.h"
#include "pf/basic/string.h"
#include "pf/basic/stringstream.h"
#include "pf/basic/monitor.h"
#include "pf/basic/io.tcc"
#include "pf/db/interface.h"
#include "pf/db/query.h"
#include "pf/net/connection/basic.h"
#include "pf/cache/packet/db_query.h"
#include "pf/sys/thread.h"
#include "pf/sys/memory/share.h"
#include "pf/engine/kernel.h"
#include "pf/cache/db_store.h"

#define cache_clear(k) {using namespace pf_sys::memory::share; \
  if (GLOBALS["default.cache.clear"] == true) clear(k); \
}

#define CACHE_SHARE_FLAG (pf_sys::memory::share::kFlagMax + 1)
#define cache_lock(c,n) \
  pf_sys::memory::share::unique_lock<db_item_t> n(*c, CACHE_SHARE_FLAG)
#define DB_ROW_MAX (1024)

namespace pf_cache {

DBStore::DBStore() : 
  service_{false},
  ready_{false},
  query_net_{false},
  db_env_{nullptr},
  net_manager_{nullptr},
  get_db_connection_func_{nullptr},
  workers_{nullptr},
  cache_last_check_time_{0},
  dbtype_{kDBTypeMysql} {
  keys_.key_map = ID_INVALID;
  keys_.recycle_map = ID_INVALID;
  keys_.query_map = ID_INVALID;
  packet_id_.query = CACHE_SHARE_NET_QUERY_PACKET_ID;
  packet_id_.result = CACHE_SHARE_NET_RESULT_PACKET_ID;
  hash_size_map_.init(100);
  recycle_size_map_.init(100);
  share_config_map_.init(100);
  forgetlist_.clear();
}

DBStore::~DBStore() {
  //do nothing.
}

bool DBStore::load_config(const std::string &file_name) {
  pf_file::Tab conf(0);
  bool ok = false;
  ok = conf.open_from_txt(file_name.c_str());
  if (!ok) return false;
  auto number = conf.get_record_number();
  for (decltype(number) i = 0; i < number; ++i) {

    //Share config.
    auto name = conf.get_fielddata(i, "index")->string_value;
    db_share_config_t share_conf;
    share_conf.size = conf.get_fielddata(i, "size")->int_value;
    share_conf.same_columns = 
      1 == conf.get_fielddata(i, "same_columns")->int_value ? true : false;
    auto save_columns = conf.get_fielddata(i, "save_columns")->string_value;
    pf_basic::string::explode(
        save_columns, share_conf.save_columns, "#", true, true);
    share_conf.no_save = 
      1 == conf.get_fielddata(i, "no_save")->int_value ? true : false;
    share_conf.save_interval = conf.get_fielddata(i, "save_interval")->int_value;
    share_conf.index = static_cast<int16_t>(conf.get_fielddata(i, "group_index")->int_value);
    share_conf.share_key = conf.get_fielddata(i, "share_key")->int_value;
    share_conf.recycle_size = static_cast<int16_t>(conf.get_fielddata(i, "recycle_size")->int_value);
    share_conf.name = name;
    share_conf.data_size = conf.get_fielddata(i, "data_size")->int_value;
    share_config_map_.add(name, share_conf);

    //Cache clear.
    if (service_) cache_clear(share_conf.share_key);

    //Group item.
    pf_sys::memory::share::group_item_t group_item;
    group_item.index = share_conf.index;
    group_item.size = share_conf.size;
    group_item.header_size = sizeof(db_table_base_t) + sizeof(db_table_info_t);
    group_item.data_size = sizeof(db_item_t) + share_conf.data_size;
    group_item.same_header = share_conf.same_columns;
    group_item.name = name;
    std::map< int32_t, 
      std::vector< pf_sys::memory::share::group_item_t > >::iterator it;
    it = share_group_map_.find(share_conf.share_key);
    if (it == share_group_map_.end()) {
      std::vector< pf_sys::memory::share::group_item_t > temp;
      temp.push_back(group_item);
      share_group_map_[share_conf.share_key] = temp;
    } else {
      it->second.push_back(group_item);
    }

    size_map_iterator it1;
    //Hash key.
    it1 = hash_size_map_.find(share_conf.share_key);
    if (it1 != hash_size_map_.end()) {
      it1->second += static_cast<int32_t>(share_conf.size);
    } else {
      hash_size_map_.add(share_conf.share_key, static_cast<int32_t>(share_conf.size));
    }
    //Recycle key.
    it1 = recycle_size_map_.find(share_conf.share_key);
    if (it1 == recycle_size_map_.end()) {
      recycle_size_map_.add(share_conf.share_key, share_conf.recycle_size);
    }
  }
  return true;
}

bool DBStore::init() {
  if (ready_) return true;
  using namespace pf_sys::memory::share;
  //Check recycle and hash key.
  if (ID_INVALID == keys_.key_map || 
      ID_INVALID == keys_.recycle_map || 
      ID_INVALID == keys_.query_map)
    return false;
  //Check config.
  if (0 == share_config_map_.getcount() || 0 == share_group_map_.size())
    return false;

  //Share pool init.
  std::map< int32_t, std::vector< group_item_t > >::iterator it;
  for (it = share_group_map_.begin(); it != share_group_map_.end(); ++it) {
    uint32_t key = static_cast<uint32_t>(it->first);
    const std::vector< group_item_t > &items = it->second;
    std::unique_ptr< SharePool > temp(new SharePool(key, items));
    if (!temp->init(service_)) {
      SLOW_ERRORLOG(CACHE_MODULENAME, 
                    "[cache] DBStore::init share pool error, key: %d", 
                    key);
      return false;
    }
    share_pool_map_[key] = std::move(temp);
  }

  size_t hash_key_count = 0;
  size_t hash_recycle_count = 0;
  size_map_iterator it1;
  for (it1 = hash_size_map_.begin(); 
       it1 != hash_size_map_.end(); 
       ++it1) {
    hash_key_count += it1->second;
  }

  for (it1 = recycle_size_map_.begin(); 
       it1 != recycle_size_map_.end(); 
       ++it1) {
    hash_recycle_count += it1->second;
  }
  //key map and recycle map init;
  uint32_t key_size = CACHE_SHARE_HASH_KEY_SIZE;
  uint32_t value_size = CACHE_SHARE_HASH_VALUE_SIZE;

  if (service_) cache_clear(keys_.key_map);
  if (!key_map_.init(
        keys_.key_map, hash_key_count, key_size, value_size, service_)) {
    SLOW_ERRORLOG(CACHE_MODULENAME, 
                  "[cache] DBStore::init hash key map failed, key: %d,"
                  " size: %d",
                  keys_.key_map,
                  hash_key_count);
    return false;
  }

  if (service_) cache_clear(keys_.recycle_map);
  if (!recycle_map_.init(
        keys_.recycle_map, hash_recycle_count, key_size, value_size, service_)) {
    SLOW_ERRORLOG(CACHE_MODULENAME, 
                  "[cache] DBStore::init hash recycle map failed, key: %d,"
                  " size: %d",
                  keys_.recycle_map,
                  hash_recycle_count);
    return false;
  }

  if (service_) cache_clear(keys_.query_map);
  if (!query_map_.init(
        keys_.query_map, hash_recycle_count, key_size, value_size, service_)) {
    SLOW_ERRORLOG(CACHE_MODULENAME, 
                  "[cache] DBStore::init hash query map failed, key: %d,"
                  " size: %d",
                  keys_.query_map,
                  hash_recycle_count);
    return false;
  }
  auto workers_count = GLOBALS["default.cache.workers"].get<int32_t>();
  auto workers = new pf_sys::ThreadPool(workers_count);
  if (is_null(workers)) return false;
  unique_move(pf_sys::ThreadPool, workers, workers_);
  ready_ = true;
  return true;
}

/**
 * cn:
 * key的存储结构为：表名#唯一key（如玩家ID）
 * key_map_中对应缓存的hash为：共享内存组内的数据索引#回收索引（相同共享内存key
 * 和唯一key下的一组缓存的回收索引应该一致）
 * 在回收中对应的key为：共享内存key#回收索引，回收索引不存在或为-1表示不回收
 * 共享内存MAP中存储的为唯一key（如玩家ID）
 **/ 
void *DBStore::get(const char *key) {
  auto item = getitem(key);
  return is_null(item) ? nullptr : item->get_data();
}

db_item_t *DBStore::getitem(const std::string &key) {
  using namespace pf_basic;
  auto ckey = key.c_str();
  if (0 == key_map_.size()) return nullptr;
  if (!cache_key_is_valid(ckey)) return nullptr;
  db_item_t *result = nullptr;
  cache_info_t info; cache_info(ckey, info);
  if (ID_INVALID == info.share_key || 
      INDEX_INVALID == info.share_index) return nullptr;
  share_config_iterator it_conf;
  it_conf = share_config_map_.find(info.name);
  if (it_conf == share_config_map_.end()) return nullptr;
  //Recycle remove.
  if (info.recycle_index != INDEX_INVALID)
    recycle_drop(info.share_key, info.recycle_index);
  share_pool_iterator it_pool;
  it_pool = share_pool_map_.find(info.share_key);
  if (it_pool == share_pool_map_.end()) return nullptr;
  result = it_pool->second->item(it_conf->second.index, static_cast<int16_t>(info.share_index));
  return result;
}

void DBStore::put(const char *key, void *value, int32_t) {
  using namespace pf_basic;
  if (!cache_key_is_valid(key)) return;
  cache_info_t info; cache_info(key, info);
  if (ID_INVALID == info.share_key) return;
  share_config_iterator it_conf;
  it_conf = share_config_map_.find(info.name);
  if (it_conf == share_config_map_.end()) {
    SLOW_ERRORLOG(CACHE_MODULENAME, 
                  "[cache] DBStore::put error, can't find config from name: %s", 
                  info.name.c_str());
    return;
  }

  share_pool_iterator it_pool;
  it_pool = share_pool_map_.find(it_conf->second.share_key);
  if (it_pool == share_pool_map_.end()) return;
  if (INDEX_INVALID == info.share_index) {
    int16_t data_index = INDEX_INVALID;
    it_pool->second->alloc(it_conf->second.index, data_index);
    if (INDEX_INVALID == data_index) {
      auto freesize = recycle_free(info.share_key, 1);
      if (freesize <= 0) return;
      it_pool->second->alloc(it_conf->second.index, data_index);
      if (INDEX_INVALID == data_index) return;
    }
    char *cache = 
      it_pool->second->real_data(it_conf->second.index, data_index);
    cache_set(cache, value, it_conf->second.data_size);
    char hash[128]{0};
    snprintf(hash, sizeof(hash) - 1, "%d#%d", data_index, -1);
    key_map_.set(key, hash);
    auto cache_item = 
      it_pool->second->item(it_conf->second.index, data_index);
    cache_item->clear();
    cache_item->size = it_conf->second.data_size;
    memcpy(cache_item->only_key, info.only_key.c_str(), info.only_key.size());
  } else { //Cached.
    char *cache = 
        it_pool->second->real_data(it_conf->second.index, static_cast<int16_t>(info.share_index));
    cache_set(cache, value, it_conf->second.data_size);
    if (info.recycle_index != INDEX_INVALID) 
      recycle_drop(info.share_key, info.recycle_index);
  }
}

//Need add the save in forget.
void DBStore::forget(const char *key) {
  using namespace pf_basic;
  if (!cache_key_is_valid(key)) return;
  cache_info_t info; cache_info(key, info);
  if (ID_INVALID == info.share_key || INDEX_INVALID == info.share_index) return;
  share_config_iterator it_conf;
  it_conf = share_config_map_.find(info.name);
  if (it_conf == share_config_map_.end()) {
    SLOW_ERRORLOG(CACHE_MODULENAME, 
                  "[cache] DBStore::forget error,"
                  " can't find config from name: %s", 
                  info.name.c_str());
    return;
  }
  share_pool_iterator it_pool;
  it_pool = share_pool_map_.find(info.share_key);
  if (it_pool == share_pool_map_.end()) return;
  query(key); //Save.
  auto tindex = it_conf->second.index;
  auto sindex = static_cast<int16_t>(info.share_index);

  //在多线程下，删除内存可能会存在解锁错误问题，因此这里的内存锁需要特别注意
  //必须保证锁在过程中不被修改
  auto item = it_pool->second->item(tindex, sindex);
  cache_lock(item, auto_lock);
  auto mutex_value = item->mutex.load();
  auto swap_index = it_pool->second->free(tindex, sindex);
  //New cache share index changed.
  if (swap_index > 0) {
    item->mutex.exchange(mutex_value);
    cache_info_t swap_info;
    char swap_key[128]{0};
    snprintf(swap_key, 
             sizeof(swap_key) - 1, 
             "%s#%s", 
             it_conf->second.name.c_str(), 
             item->only_key);
    cache_info(swap_key, swap_info);
    char new_hash[128]{0};
    snprintf(new_hash, 
             sizeof(new_hash) - 1,
             "%d#%d", 
             info.share_index, swap_info.recycle_index);
    key_map_.set(swap_key, new_hash);
  } else {
    recycle_remove(info.share_key, info.only_key, key);
  }
  key_map_.remove(key);
}

void DBStore::forgetall(const std::string &only_key) {
  auto it = share_config_map_.begin();
  for ( ;it != share_config_map_.end(); ++it) {
    std::string key{""};
    key = it->first;
    key += only_key;
    this->forget(key.c_str());
  }
}

bool DBStore::cache_key_is_valid(const char *key) {
  using namespace pf_basic;
  //Check key is valid.
  if (is_null(key)) return false;
  std::vector< std::string > array;
  string::explode(key, array, "#", true, true);
  if (array.size() != 2) return false;
  return true;
}

size_t DBStore::recycle_free(int32_t key, size_t size) {
  size_t realsize{0};
  if (0 == size) {
    char count_key[128]{0};
    snprintf(count_key, sizeof(count_key) - 1, "count_%d", key);
    const char *temp = recycle_map_[count_key];
    pf_basic::type::variable_t count = is_null(temp) ? 0 : temp;
    size = static_cast<size_t>(count.get<int32_t>());
  }
  for (size_t i = 0; i < size; ++i) {
    std::string only_key{""};
    auto pop = recycle_pop(key, only_key);
    if (pop) {
      ++realsize; forgetall(only_key);
    } else {
      break;
    }
  }
  return realsize;
}

bool DBStore::recycle_pop(int32_t key, std::string &only_key) {
  using namespace pf_basic::type;
  char count_key[128]{0};
  snprintf(count_key, sizeof(count_key) - 1, "count_%d", key);
  const char *temp = recycle_map_[count_key];
  variable_t count = is_null(temp) ? 0 : temp;
  if (count.get<int32_t>() <= 0) return false;
  char first_key[128]{0};
  snprintf(first_key, sizeof(first_key) - 1, "%d_0", key);
  variable_t val{0};
  only_key = recycle_map_[first_key];
  //Swap last key to first.
  char last_key[128]{0};
  snprintf(
      last_key, sizeof(last_key) - 1, "%d_%d", key, count.get<int32_t>() - 1);
  recycle_map_.set(first_key, recycle_map_[last_key]);
  recycle_map_.set(last_key, 0);
  val = count.get<int32_t>() - 1;
  recycle_map_.set(count_key, val.c_str());
  return true;
}

bool DBStore::recycle_push(int32_t key, 
                           const std::string &only_key) {
  using namespace pf_basic::type;
  char count_key[128]{0};
  snprintf(count_key, sizeof(count_key) - 1, "count_%d", key);
  const char *temp = recycle_map_[count_key];
  variable_t count = is_null(temp) ? 0 : temp;
  if (!recycle_size_map_.isfind(key)) {
    SLOW_ERRORLOG(CACHE_MODULENAME,
                  "[cache] DBStore::recycle_drop error,"
                  " can't find the recycle size key: %d",
                  key);
    return false;
  }
  auto size = recycle_size_map_.get(key);
  if (count.get<int32_t>() >= size) {
#if _DEBUG
    pf_basic::io_cdebug("key: %d, recycle full, size: %d", key, size);
#endif
    return false;
  }
  auto index = count.get<int32_t>();
  count += 1;
  recycle_mod(key, only_key, index);
  return true;
}

void DBStore::recycle_remove(int32_t key, 
                             const std::string &only_key, 
                             const std::string &except_key) {
  using namespace pf_basic::type;
  std::map< 
    int32_t, std::vector< pf_sys::memory::share::group_item_t > >::iterator it;
  it = share_group_map_.find(key);
  if (it == share_group_map_.end()) {
    SLOW_ERRORLOG(CACHE_MODULENAME, 
                  "[cache] DBStore::recycle_remove error,"
                  " can't find config from key: %d",
                  key);
    return;
  }
  char temp[128]{0};
  std::vector< pf_sys::memory::share::group_item_t > &items = it->second;
  for (size_t i = 0; i < items.size(); ++i) {
    memset(temp, 0, sizeof(temp));
    snprintf(temp, 
             sizeof(temp) - 1, 
             "%s#%s", 
             items[i].name.c_str(), 
             only_key.c_str());
    if (except_key != temp) forget(temp);
  }
}

//改变回收索引，会将此only key下所有的缓存hash更改为指定索引。
void DBStore::recycle_mod(
    int32_t key, const std::string &only_key, int32_t index) {
  using namespace pf_basic;
  group_map_iterator it;
  it = share_group_map_.find(key);
  if (it == share_group_map_.end()) {
    SLOW_ERRORLOG(CACHE_MODULENAME,
                  "[cache] DBStore::recycle_mod error,"
                  " can't find group config from key: %d",
                  key);
    return;
  }
  for (size_t i = 0; i < it->second.size(); ++i) {
    const pf_sys::memory::share::group_item_t &item = it->second[i];
    char hash_key[128]{0};
    snprintf(hash_key, 
             sizeof(hash_key) - 1, 
             "%s#%s", 
             item.name.c_str(), only_key.c_str());
    std::vector< std::string > array;
    const char *hash = key_map_[hash_key];
    string::explode(hash, array, "#", true, true);
    char new_hash[128]{0};
    snprintf(new_hash, sizeof(new_hash) - 1, "%s#%d", array[0].c_str(), index);
    key_map_.set(hash_key, new_hash);
  }
}

void DBStore::recycle_drop(int32_t key, int32_t index) {
  using namespace pf_basic;
  group_map_iterator it;
  it = share_group_map_.find(key);
  if (it == share_group_map_.end()) {
    SLOW_ERRORLOG(CACHE_MODULENAME,
                  "[cache] DBStore::recycle_drop error,"
                  " can't find group config from key: %d",
                  key);
    return;
  }

  //Check recycle is valid.
  char count_key[128]{0};
  snprintf(count_key, sizeof(count_key) - 1, "count_%d", key);
  const char *count_hash = recycle_map_[count_key];
  type::variable_t count = is_null(count_hash) ? 0 : count_hash;
  if (index > count.get<int32_t>() - 1) return;
  char delete_recycle_key[128]{0};
  snprintf(delete_recycle_key, 
           sizeof(delete_recycle_key) - 1, 
           "%d_%d", 
           key, index);
  const char *delete_only_key = recycle_map_[delete_recycle_key];
  if (nullptr == delete_only_key) return;
  type::variable_t val{""};
  val = delete_only_key;
  if (val == "" || val.get<int32_t>() == 0) return;

  //Recycle hash.
  auto swap_index = count.get<int32_t>() - 1; 
  char swap_recycle_key[128]{0};
  snprintf(swap_recycle_key, 
           sizeof(swap_recycle_key) - 1, 
           "%d_%d", 
           key, swap_index);
  const char *swap_only_key = recycle_map_[swap_recycle_key];

  //Hash key swap all.
  recycle_mod(key, delete_only_key, -1);
  recycle_mod(key, swap_only_key, index);

  //Dec size and swap.
  val = count.get<int32_t>() - 1;
  key_map_.set(count_key, val.c_str());
  if (index == count.get<int32_t>() - 1) {
    key_map_.set(delete_recycle_key, "0");
    return;
  }
  //Recycle swap.
  key_map_.set(delete_recycle_key, swap_only_key); 
  key_map_.set(swap_recycle_key, "0");
}

void DBStore::cache_info(const char *key, cache_info_t &cache_info) {
  using namespace pf_basic;
  type::variable_t val;
  //Key.
  std::vector< std::string > array;
  string::explode(key, array, "#", true, true);
  if (array.size() != 2) return;
  cache_info.name = array[0];
  cache_info.only_key = array[1];
  share_config_iterator it_conf;
  it_conf = share_config_map_.find(cache_info.name);
  if (it_conf == share_config_map_.end()) return;
  cache_info.share_key = it_conf->second.share_key;

  array.clear();

  //Index.
  const char *hash = key_map_[key];
  if (is_null(hash)) return;
  string::explode(hash, array, "#", true, true);
  if (array.size() != 2) return;
  val = array[0];
  cache_info.share_index = val.get<int32_t>();
  val = array[1];
  cache_info.recycle_index = val.get<int32_t>();
}

bool DBStore::recycle_find(int32_t key, int32_t index) {
  using namespace pf_basic;
  char recycle_key[128]{0};
  snprintf(recycle_key, sizeof(recycle_key) - 1, "%d_%d", key, index);
  const char *hash = recycle_map_[recycle_key];
  type::variable_t val;
  val = is_null(hash) ? 0 : hash;
  return val != "" && val != 0;
}

#define hash_common(k,p,f) using namespace pf_basic; \
  auto ckey = (k).c_str(); \
  auto cache = getitem(k); \
  if (is_null(cache) && (p)) { \
    this->forever(ckey, cast(void *, "")); \
    cache = getitem(k); \
  } \
  if (is_null(cache)) return false;\
  cache_info_t info; \
  cache_info(ckey, info); \
  auto it_pool = share_pool_map_.find(info.share_key); \
  if (it_pool == share_pool_map_.end()) { f(cache); return false; } \
  auto it_conf = share_config_map_.find(info.name); \
  if (it_conf == share_config_map_.end()) { f(cache); return false; }\
  auto table_info = \
    it_pool->second->table_info(it_conf->second.index, static_cast<int16_t>(info.share_index)); \
  if (is_null(table_info)) { f(cache); return false; } \
  auto data = cache->get_data();

#define cache_error(c) (c)->status = kQueryError

//Future will optimize by query type(select insert delete...)
//and need protected with multi threads.
//The update query can record the change status to update to sql.
bool DBStore::query(const std::string &key) {
  hash_common(key, false, cache_error);
  cache_lock(cache, cachelock);
  std::string sql{""};
  auto status = cache->status;
  auto db_connection = query_net_ && get_db_connection_func_ ? 
    get_db_connection_func_(*cache) : nullptr;
  auto db_env = db_env_;
  if (is_null(db_connection) && is_null(db_env) && ENGINE_POINTER) {
    db_env = ENGINE_POINTER->get_db();
  }
  if (is_null(db_connection) && is_null(db_env)) {
    cache_error(cache);
    return false;
  }
  if (!generate_sql(key, sql) || "" == sql) {
    cache_error(cache);
    return false;
  }
  if (db_connection) {
    packet::DBQuery packet;
    packet.set_type(cache->status); //Query status.
    packet.set_id(packet_id_.query);
    packet.set_sql_str(sql.c_str());
    return db_connection->send(&packet);
  } else {
    cache->status = kQueryError;
    pf_db::Query query;
    query.set_sql(sql);
    db_lock(db_env, db_auto_lock);
    if (!query.init(db_env) || !query.query()) {
      return false;
    }
    if (kQuerySelect == status && !query.fetch(
          cast(char *, table_info), 
          sizeof(db_table_info_t), 
          data, 
          cache->size)) return false;
    cache->status = kQuerySuccess;
  }
  return kQuerySuccess == cache->status ? true : false;
}

bool DBStore::waitquery(const char *key) {
  if (query_map_.full()) return false;
  query_map_.set(key, "1");
  return true;
}

bool DBStore::get(const std::string &key, db_fetch_array_t &hash) {
  hash_common(key, false, (void));
  if (is_null(data)) return false;
  std::vector< int8_t > types;
  stringstream scolumns(cast(char *, table_info), sizeof(db_table_info_t));
  int32_t column_count{0};
  scolumns >> column_count;
  if (0 == column_count) return false;
  for (size_t i = 0; i < static_cast<size_t>(column_count); ++i) {
    std::string name{""};
    int8_t type{kDBColumnTypeString};
    scolumns >> name;
    scolumns >> type;
    hash.keys.push_back(name);
    types.push_back(type);
  }
  int32_t row{0};
  stringstream srows(data, cache->size);
  srows >> row;
  if (0 == row) return true;
  for (size_t i = 0; i < static_cast<size_t>(row); ++i) {
    for (size_t j = 0; j < static_cast<size_t>(column_count); ++j) {
      auto type = types[j];
      switch (type) {
        case kDBColumnTypeInteger: {
          int64_t var{0}; srows >> var;
          type::variable_t _var{var};
          hash.values.push_back(_var);
          break;
        }
        case kDBColumnTypeNumber: {
          double var{0}; srows >> var;
          type::variable_t _var{var};
          hash.values.push_back(_var);
          break;
        }
        default: {
          std::string var{0}; srows >> var;
          type::variable_t _var{var};
          hash.values.push_back(_var);
          break;
        }
      }
    } //for
  } //for
  return true;
}

bool DBStore::get(const std::string &key, char *&columns, char * &rows) {
  hash_common(key, false, (void));
  if (is_null(data)) return false;
  columns = cast(char *, table_info);
  rows = data;
  return true;
}

bool DBStore::set(const std::string &key, 
                  const std::string &columns, 
                  const std::string &rows) {
  hash_common(key, false, (void));
  if (is_null(data)) return false;
  auto _columns = cast(char *, table_info);
  memcpy(_columns, columns.c_str(), columns.size());
  memcpy(data, rows.c_str(), rows.size());
  return true;
}
   
bool DBStore::set(const std::string &key, const db_fetch_array_t &hash) {
  using namespace pf_basic;
  hash_common(key, true, (void));
  if (!hash_is_valid(hash, cache->size)) return false;
  stringstream scolumns(cast(char*, table_info), sizeof(db_table_info_t));
  auto columns_count = static_cast<int32_t>(hash.keys.size());
  scolumns << columns_count;
  for (size_t i = 0; i < hash.keys.size(); ++i) {
    scolumns << hash.keys[i].c_str();
    scolumns << hash.values[i].type;
  }
  stringstream srows(data, cache->size);
  int32_t row = 0;
  auto row_position = srows.get_position();
  srows << row;
  for (size_t i = 0; i < hash.values.size(); ++i) {
    switch (hash.values[i].type) {
      case type::kVariableTypeInt64:
        srows << hash.values[i].get<int64_t>();
        break;
      case type::kVariableTypeDouble:
        srows << hash.values[i].get<double>();
        break;
      default:
        srows << hash.values[i].c_str();
        break;
    }
    if (srows.full()) return false;
    if (i != 0 && i % columns_count == 0) row += 1;
  }
  srows.set_position(static_cast<int32_t>(row_position));
  srows << row;
  return true;
}

bool DBStore::generate_sql(const std::string &key, std::string &sql) {
  using namespace pf_basic; auto ckey = key.c_str();
  cache_info_t info; cache_info(ckey, info);
  auto it_pool = share_pool_map_.find(info.share_key);
  auto it_conf = share_config_map_.find(info.name);
  if (it_pool == share_pool_map_.end() || it_conf == share_config_map_.end())
    return false;
  auto tindex = it_conf->second.index;
  auto sindex = static_cast<int16_t>(info.share_index);
  //auto t_base = it_pool->second->table_base(tindex, sindex);
  auto t_info = it_pool->second->table_info(tindex, sindex);
  auto t_item = it_pool->second->item(tindex, sindex);
  if (is_null(t_info) || is_null(t_item)) return false;
  if (kQueryInvalid == t_item->status || 
      kQueryError == t_item->status || 
      kQueryWaiting == t_item->status) return false;
  pf_db::Query query;
  /**
  std::string table_name;
  table_name = t_base->prefix;
  table_name += t_base->name;
  **/
  switch (t_item->status) {
    case kQuerySelect:
    case kQueryInsert:
    case kQueryDelete:
      sql += t_item->get_data();
      break;
    default: { //kQueryUpdate
      type::variable_array_t names;
      std::map<std::string, int32_t> hnames; //Hash to index.
      std::vector< int8_t > types;
      stringstream scolumns(cast(char *, t_info), sizeof(db_table_info_t));
      int32_t column_count{0};
      scolumns >> column_count;
      //pf_basic::io_cdebug("gs scolumns: %d key: %s", column_count, key.c_str());
      if (0 == column_count) {
        break;
      }
      for (size_t i = 0; i < static_cast<size_t>(column_count); ++i) {
        std::string name{""};
        int8_t type{kDBColumnTypeString};
        scolumns >> name;
        scolumns >> type;
        names.push_back(name);
        types.push_back(type);
        hnames[name] = static_cast<int32_t>(i);
      }
      int32_t row{0};
      stringstream srows(t_item->get_data(), t_item->size);
      srows >> row;
      char msg[1024]{0};
      snprintf(msg, sizeof(msg) - 1, "[%s|%d]", key.c_str(), row);
      AssertEx(row >= 0 && row <= DB_ROW_MAX, msg);
      if (row <= 0 || row > DB_ROW_MAX) return false;
      query.set_tablename(info.name.c_str());
      type::variable_array_t values;
      std::vector< std::string > &save_columns = it_conf->second.save_columns;
      for (decltype(row)i = 0; i < row; ++i) {
        for (decltype(column_count)j = 0; j < column_count; ++j) {
          if (srows.full()) break;
          auto type = types[j];
          switch (type) {
            case kDBColumnTypeInteger: {
              int64_t var{0}; srows >> var;
              type::variable_t _var{var};
              values.push_back(_var);
              break;
            }
            case kDBColumnTypeNumber: {
              double var{0}; srows >> var;
              type::variable_t _var{var};
              values.push_back(_var);
              break;
            }
            default: {
              std::string var{0}; srows >> var;
              type::variable_t _var{var};
              values.push_back(_var);
              break;
            }
          }
        }
      }
      if (values.size() > names.size()) {
        query.update(names, values, dbtype_);
      } else {
          query.update(names, values);
          std::string save_cond;
          for (size_t m = 0; m < save_columns.size(); ++m) {
            char temp[128]{0,};
            auto name = save_columns[m].c_str();
            auto index = hnames[name];
            snprintf(temp, 
                     sizeof(temp) - 1, 
                     kDBColumnTypeString == types[index]
                     ? "%s = \'%s\'" : "%s = %s", 
                     name, 
                     values[hnames[name]].c_str());
            save_cond += temp;
            if (m != 0 && m != save_columns.size())
              save_cond += " and ";
          }
          save_cond += ";";
          query.where(save_cond.c_str());
      }
      query.get_sql(sql);
      break;
    }
  }
  return true;
}

bool DBStore::hash_is_valid(const db_fetch_array_t &hash, size_t size) {
  db_keys_t::iterator _iterator;
  if (hash.values.size() != 0) {
    if ((hash.values.size() % hash.keys.size()) != 0) return false;
    if (hash.size() < 1) return false;
  }
  auto key_size = hash.keys.size();
  size_t key_length = 0;
  for (size_t i = 0; i < key_size; ++i) {
    if (!is_null(strstr("\t", hash.keys[i].c_str()))) return false;
    key_length += strlen(hash.keys[i].c_str());
    key_length += sizeof(int8_t);
  }
  if (key_length > CACHE_DB_TABLE_COLUMNS_SIZE - 1) return false;
  size_t data_length = 0;
  for (size_t i = 0; i < hash.values.size(); ++i) {
    if (!is_null(strstr("\t", hash.values[i].c_str()))) return false;
    data_length += strlen(hash.values[i].c_str());
  }
  return data_length < size;
}

void DBStore::flush() {
  using namespace pf_sys::memory;
  if (!service_) return;
  share::map_iterator it;
  for (it = key_map_.begin(); it != key_map_.end(); ++it)
    forget(it.first);
}

void DBStore::tick() {
  using namespace pf_sys::memory;
  if (!service_ || !ready_) return;

  //For waiting query.
  {
    if (query_map_.size() > 0) {
      std::string key{""}; std::string value{""};
      query_map_.pop_front(key, value);
      workers_->enqueue([this, key](){ this->query(key); });
    }
  }

  //For forget.
  {
    if (!forgetlist_.empty()) {
      std::string key = forgetlist_.back();
      forgetlist_.pop_back();
      workers_->enqueue([this, key](){ 
          this->query(key); this->forget(key.c_str()); });
    }
  }

  //Check live time.
  static uint32_t check_time = CACHE_SHARE_DEFAULT_MINUTES * 60;
  auto current_time = TIME_MANAGER_POINTER->get_current_time();
  if (forgetlist_.empty() && 
      current_time - cache_last_check_time_ > check_time) {
    cache_last_check_time_ = current_time;
    auto it = key_map_.begin();
    for (; it != key_map_.end(); ++it) {
      auto cache = getitem(it.first);
      if (cache && cache->hook_time > 0 && cache->hook_time < current_time) {
        forgetlist_.push_back(it.first);
      }
    }
  }
}

} //namespace pf_cache
