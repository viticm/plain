/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id define.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2016/09/30 14:41
 * @uses cache define file
 *       数据库的表的存储方式（以配置的same_columns为准）
 *       1、
 *         暂时定义为常驻缓存（缓存周期很长或无限）
 *         
 *         一般情况下，不以查询为目的的情况下，
 *         应该读取查询条件下整张表（或者固定字段）的内容
 *         
 *         那么缓存的结构为：[tableinfo|db_item_struct|datas]
 *         
 *         tableinfo为 db_table_base_struct|db_table_info_struct
 *         datas为查询的结果数据 为[长度信息|数据]构成的集合
 *
 *         --------------------------------------------------------------
 *         这里分为不同key下存储多张表或一张表的问题，特别在有些系统中，
 *         许多表的数据的拥有者为同一个对象时，这样做更有效地进行管理
 *
 *         那么每个共享内存则为上述结构的合集，需要按照表名顺序存储
 *         缓存结构为：[(tableinfo|db_item_struct...)|(datas...)]
 *       2、
 *         暂时定义为临时缓存（缓存周期很短或者查询后立即删除）
 *         
 *         其次以查询为目录的缓存，可以自定义查询的字段
 *         
 *         缓存结构为：[tableinfo|fulldatas]
 *         
 *         其中flag的值为 0
 *         tableinfo为 db_table_base_struct
 *         fulldatas 为[db_table_info_struct|db_item_struct|datas]
 *         datas同上
 */
#ifndef PF_CACHE_DEFINE_H_
#define PF_CACHE_DEFINE_H_

#include "pf/cache/config.h"
#include "pf/sys/memory/share.h"
#include "pf/db/define.h"
#include "pf/basic/type/variable.h"

#define CACHE_DBNODE_MAX 20
#define CACHE_SHARE_POOLSIZE 1000
#define CACHE_SHARE_KEYSIZE 30
#define CACHE_DB_TABLE_COLUMNS_SIZE      (1024)     //列信息的总长度
#define CACHE_DB_ITEM_LENGTHS_SIZE       (256)      //数据大小总长度
#define CACHE_SHARE_HASH_KEY_SIZE        (128) 
#define CACHE_SHARE_HASH_VALUE_SIZE      (64)
#define CACHE_SHARE_DEFAULT_MINUTES      (10)       //默认缓存的分钟数
#define CACHE_SHARE_NET_QUERY_PACKET_ID  (0xF001)   //默认查询的网络包ID
#define CACHE_SHARE_NET_RESULT_PACKET_ID (0xF002)   //默认结果的网络包ID

namespace pf_cache {

enum {                                                                             
  kQueryInvalid = -1,     //默认状态，不用任何操作                                                         
  kQueryError,            //错误状态                                                          
  kQueryWaiting,          //等待状态                                                        
  kQueryInsert,           //等待数据库初始化                                                 
  kQueryDelete,           //等待数据库删除                                                   
  kQueryUpdate,           //等待数据库更新                                                   
  kQuerySelect,           //等待数据库查询                                                   
  kQuerySuccess,          //执行成功，不进行任何操作                                        
}; //数据库查询的类型，增、删、改、查，同时也标记着缓存的数据库状态                


struct db_table_base_struct {
  //The name of the cache table.
  char name[DB_TABLENAME_LENGTH];

  //A string that should be prepended to keys.
  char prefix[DB_PREFIX_LENGTH];

  db_table_base_struct() :
    name{0},
    prefix{0} {}
};

struct db_table_info_struct {

  //Database column info(name,type|name1,type1...).
  char columns[CACHE_DB_TABLE_COLUMNS_SIZE];
 
  db_table_info_struct() : 
    columns{0} {}
};

struct db_item_struct {

  //The status of this item.
  int8_t status;

  //The type of this item.
  uint8_t type;

  //The item size.
  size_t size;

  //The only key.
  char only_key[64];

  //The hook time.
  uint32_t hook_time; // < 0 is the forever cache.

  //The param array.
  int32_t param[3];

  //The share mutex.
  pf_sys::memory::share::mutex_t mutex;

  char *get_data() {
    return reinterpret_cast<char *>(this) + sizeof(db_item_struct);
  }
  void lock(int8_t flag) {
    pf_sys::memory::share::lock(mutex, flag);
  }
  void unlock(int8_t flag) {
    pf_sys::memory::share::unlock(mutex, flag);
  }

  void clear() {
    mutex.exchange(pf_sys::memory::share::kFlagFree);
    status = kQueryInvalid;
    size = 0;
    memset(only_key, 0, sizeof(only_key));
    hook_time = 0;
    memset(param, 0, sizeof(param));
  }

  /**
  //Like database fetch array, cache data to it.
  bool data_to_fetch_array(
      db_fetch_array_t &array, db_table_info_struct *info);

  //Fetch array to cache data.
  bool fetch_array_to_data(
      const db_fetch_array_t &array, db_table_info_struct *info);

  //Check fetch array is valid to cache data.
  bool is_valid_fetch_array(
      const db_fetch_array_t &array, db_table_info_struct *info);
  **/

  db_item_struct() :
    status{kQueryInvalid},
    size{0},
    only_key{0},
    hook_time{0},
    mutex{pf_sys::memory::share::kFlagFree} {}
};

//The share item config for one T(table).
struct db_share_config_struct {

 //One table use same columns.
 bool same_columns;

 //No save.
 bool no_save;

 //表名
 std::string name;

 //保存的列名
 std::vector< std::string > save_columns;

 //Data size(not include the tableinfo and conditions).
 //数据大小，一般情况下指的是从数据表中查询出的数据可能的最大值长度。
 size_t data_size;

 //Share key.
 int32_t share_key;

 //Size.
 size_t size;

 //Index, if many tables use one share key.
 int16_t index;

 //Recycle size, 0 then the cache not recycle and store forever.
 int16_t recycle_size;

 //Save interval.
 int32_t save_interval;

 db_share_config_struct() :
   same_columns{true},
   no_save{false},
   //condition_size{0},
   name{""},
   //condition{""},
   data_size{0},
   share_key{-1},
   size{0},
   index{0},
   recycle_size{0},
   save_interval{0} {}

};

//The db table columns info.
struct db_table_cinfo_struct {
  std::vector<std::string> names;
  std::vector<int8_t> types;
};

//Call back function for store.
typedef void *(__stdcall *function_callback)();

//DB table base struct.
using db_table_base_t = struct db_table_base_struct;

//DB table info struct.
using db_table_info_t = struct db_table_info_struct;

//DB save item struct.
using db_item_t = struct db_item_struct;

//DB share config struct.
using db_share_config_t = struct db_share_config_struct;

//DB table columns info.
using db_table_cinfo_t = struct db_table_cinfo_struct;

}; //namespace pf_cache

//Some useful macros.
#define cast_dcache(v)cast(db_item_t *, (v)) //dcache is db cache.
#define dcache_key(n, k, r)(r) += (n);(r) += "#";(r) += (k);

#endif //PF_CACHE_DEFINE_H_
