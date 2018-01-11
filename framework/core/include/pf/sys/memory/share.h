/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id share.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/21 23:05
 * @uses system share memory model
 *       cn: 这个类需要重新设计，以简洁易懂为原则加上C++11的一些优美写法
 */
#ifndef PF_SYS_MEMORY_SHARE_H_
#define PF_SYS_MEMORY_SHARE_H_

#include "pf/sys/memory/config.h"
#include "pf/basic/time_manager.h"
#include "pf/basic/logger.h"

namespace pf_sys {

namespace memory {

namespace share {

//Type defines.
using mutex_t = std::atomic<int8_t>;
using header_t = struct header_struct;
using dataheader_t = struct dataheader_struct;
typedef bool (__stdcall *function_node_save)(void *, void *);

namespace api {

#if OS_UNIX
PF_API int32_t create(uint32_t key, size_t size);
PF_API int32_t open(uint32_t key, size_t size, bool errorlog = true);
PF_API void close(int32_t handle);
PF_API char *map(int32_t handle);
#elif OS_WIN
PF_API HANDLE create(uint32_t key, size_t size);
PF_API HANDLE open(uint32_t key, size_t size, bool errorlog = true);
PF_API void close(HANDLE handle);
PF_API char *map(HANDLE handle);
#endif
PF_API void unmap(char *pointer);

}; //namespace api

class PF_API Base {

 public:
   Base();
   ~Base();

 public:
   bool create(uint32_t key, size_t size);
   void release();
   bool attach(uint32_t key, size_t size, bool errorlog = true);
   char *get() { return data_; };
   char *get(uint32_t index, size_t size);
   header_t *header() { return reinterpret_cast<header_t *>(header_); };
   bool dump(const char *filename);
   bool merge(const char *filename);
   size_t size() const { return size_; };

 private:
   size_t size_;
   char *data_;
   char *header_;
#if OS_UNIX
   int32_t handle_;
#elif OS_WIN
   HANDLE handle_;
#endif

};

struct PF_API header_struct {
  uint32_t key;
  size_t size;
  uint32_t version;
  uint32_t pool_position;
  mutex_t mutex;
  header_struct();
  ~header_struct();
  void clear();
  void lock(int8_t flag);
  void unlock(int8_t flag);
};

PF_API void lock(mutex_t &mutex, int8_t type);
PF_API void unlock(mutex_t &mutex, int8_t type);

template <typename T>
class unique_lock {

 public:
   explicit unique_lock(T &m, int8_t flag) 
     : m_(m), flag_(flag) {
     m_.lock(flag_);
   };
   ~unique_lock() {
     m_.unlock(flag_);
   };

 private:
   T &m_;
   int8_t flag_;

 private:
   explicit unique_lock(unique_lock &);
   unique_lock &operator = (unique_lock &);

};

struct PF_API dataheader_struct {
  uint32_t key;
  int8_t usestatus;
  mutex_t mutex;
  uint32_t version;
  int32_t pool_id;
  dataheader_struct();
  ~dataheader_struct();
  void clear();
  dataheader_t &operator = (const dataheader_t &object);
  dataheader_t *operator = (const dataheader_t *object);
};

template <typename T>
struct data_template {

 public:
   data_template() : extend_size(0) {};
   ~data_template() {}

 public:
   dataheader_t header;
   size_t extend_size;
   T data;

 public:
   void lock(int8_t flag) { share::lock(header.mutex, flag); };
   void unlock(int8_t flag) { share::unlock(header.mutex, flag); };
   void set_pool_id(int32_t id) { header.pool_id = id; };
   int32_t get_pool_id() const { return header.pool_id; };
   void set_usestatus(int8_t status, int8_t flag);
   int8_t get_usestatus(int8_t flag);
   int8_t single_usestatus() const { return header.usestatus; };
   size_t get_extend_size() const { return extend_size; };
   void set_extend_size(size_t size) { extend_size = size; };
   uint32_t key() const { return header.key; };
   void set_key(uint32_t _key) { header.key = _key; };
   void init() { clear(); };
   void clear() {
     header.clear();
     data.clear();
   };
   void header_clear() { header.clear(); };
   data_template &operator = (const data_template &object);
   data_template *operator = (const data_template *object);
};

template <typename T>
class UnitPool {

 public:
   UnitPool();
   ~UnitPool();

 public:
   bool init(uint32_t key, size_t size, uint8_t type = kSmptDefault);
   void release();
   T *new_obj();
   T *get_obj(int32_t index);
   void delete_obj(T *obj);
   size_t size() const { return size_; };
   uint32_t key() const { return key_; };
   bool dump(const char *filename);
   bool merge(const char *filename);
   bool full() const { return size_ - 1 == get_position(); };
   uint32_t get_version() const;
   void set_version(uint32_t version);
   void set_data_extend_size(size_t _size) { data_extend_size_ = _size; };
   size_t get_data_extend_size() const { return data_extend_size_; };
   void set_header_extend_size(size_t _size) { header_extend_size_ = _size; };
   size_t get_header_extend_size() const { return header_extend_size_; } ;
   int32_t get_position() const;
   void set_position(int32_t position);
   header_t *get_header();

 protected:
   T ** objs_;
   std::unique_ptr<Base> ref_obj_pointer_;
   size_t size_;
   uint32_t key_;
   size_t data_extend_size_;    //Data extend size, unit real size: sizeof(T) + extend_size
   size_t header_extend_size_;  //Header extend size, node is sizeof(header_t) + 
                                //header_extend_size_ + 
                                //(sizeof(T) + data_extend_size_) * size
   bool ready_;

};

//[header|group0,group1]


//One item size:
//same_header: sizeof(group_item_header_t) + header_size + size * data_size;
//not same_header: sizeof(group_item_header_t) + (header_size + data_size) * size;
struct group_item_struct {
  
  int16_t index;
  
  size_t size; //Data count.

  uint32_t position;

  size_t header_size;

  size_t data_size;

  bool same_header;

  std::string name; //The group item name.

  group_item_struct() : 
    index{0},
    size{0},
    position{0},
    header_size{0},
    data_size{0},
    same_header{true},
    name{""} {}
};

struct group_header_struct {

  uint8_t flag;

  group_header_struct() : 
    flag{0} {}
};

struct group_item_header_struct {

  //The item memory position.
  int16_t pool_position;

  //Lock.
  mutex_t mutex;

  //Save version.
  uint32_t version;

  //Status.
  int8_t status;

  void lock(int8_t type) {
    share::lock(mutex, type);
  };
  void unlock(int8_t type) {
    share::unlock(mutex, type);
  };

  void clear() {
    pool_position = 0;
    mutex.exchange(kFlagFree);
    version = 0;
    status = 0;
  }

  group_item_header_struct() :
    pool_position{0},
    mutex{kFlagFree},
    version{0},
    status{0} {}
};

using group_item_t = struct group_item_struct;
using group_header_t = struct group_header_struct;
using group_item_header_t = struct group_item_header_struct;

class GroupPool {

 public:
   explicit GroupPool(uint32_t key, const std::vector<group_item_t> &group);
   ~GroupPool() {}

 public:
   bool init(bool create);
   char *get_data(int16_t index);
   group_header_t *header();
   uint32_t key() const { return key_; };
   size_t size() const { return size_; };
   group_item_header_t *item_header(int16_t index);
   char *item_data_header(int16_t index, int16_t data_index);
   char *item_data(int16_t index, int16_t data_index);
   int32_t item_data_postion(int16_t index, int16_t data_index);

 public:
   //Free all.
   void free();

 public:  //分配与归还内存块，根据每组数据实现
   char *alloc(int16_t index, int16_t &data_index);
   int16_t free(int16_t index, int16_t data_index); //return swap index.

 private:
   bool is_valid_index(int16_t index) {
     return group_conf_.find(index) != group_conf_.end();
   };

 private:
   std::map< int16_t, group_item_t > group_conf_;
   std::unique_ptr< Base > ref_obj_pointer_;
   uint32_t key_;
   size_t size_; //Full memory size.
   bool ready_;
};

template <typename T>
class Node {

 public:
   Node();
   ~Node();

 public:
   bool init(uint32_t key, size_t size);
   void clear();
   bool init_after();

 public:
   bool empty() const;
   bool full() const;
   size_t size() const;
   size_t max_size() const;
   size_t hold_count() const;
   uint32_t key() const { return key_; };
   UnitPool<T> *pool_ptr() { return pool_; };

 public:
   T *get(uint32_t index);
   T *get();
   void release();
   void set_pool_position(uint32_t position);
   void set_read_flag(int8_t flag) { read_flag_ = flag; };
   void set_write_flag(int8_t flag) { write_flag_ = flag; };
   void set_recover(bool recover) { recover_ = recover; };
   void set_minutes(uint32_t index, uint32_t minutes);

 public:
   template <class F, class... Args>
   auto handle(F&& f, Args&&... args)
   -> std::future<typename std::result_of<F(Args...)>::type>;
   
 private:
   bool ready_;
   int8_t read_flag_;
   int8_t write_flag_;
   std::shared_ptr< UnitPool< T > > pool_;
   uint32_t key_;
   bool recover_;

};

}; //namespace share

}; //namespace memory

}; //namespace pf_sys

#include "pf/sys/memory/share.tcc"

#ifndef share_lock
#define share_lock(t,o,n,f) pf_sys::memory::share::unique_lock<t> n(o, f)
#endif

#endif //PF_SYS_MEMORY_SHARE_H_
