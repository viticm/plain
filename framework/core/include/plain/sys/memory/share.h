/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id share.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/06 21:39
 * @uses system share memory model
 *       FIXME: rewrite with modern c++
 */
#ifndef PLAIN_SYS_MEMORY_SHARE_H_
#define PLAIN_SYS_MEMORY_SHARE_H_

#include "plain/sys/memory/config.h"
#include <unordered_map>
#include <utility>
#include "plain/basic/time.h"
#include "plain/basic/logger.h"
#include "plain/basic/global.h"
#include "plain/sys/assert.h"
#include "plain/sys/utility.h"

namespace plain::memory::share {

//Type defines.
using mutex_t = std::atomic<int8_t>;
using header_t = struct header_struct;
using dataheader_t = struct dataheader_struct;
using node_save_func_t = std::function<bool(void*, void*)>;

namespace api {

#if OS_UNIX
PLAIN_API int32_t create(uint32_t key, size_t size);
PLAIN_API int32_t open(uint32_t key, size_t size, bool errorlog = true);
PLAIN_API void close(int32_t handle);
PLAIN_API char *map(int32_t handle);
#elif OS_WIN
PLAIN_API HANDLE create(uint32_t key, size_t size);
PLAIN_API HANDLE open(uint32_t key, size_t size, bool errorlog = true);
PLAIN_API void close(HANDLE handle);
PLAIN_API char *map(HANDLE handle);
#endif
PLAIN_API void unmap(char *pointer);

} //namespace api

class PLAIN_API Base {

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

struct PLAIN_API header_struct {
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

PLAIN_API void lock(mutex_t &mutex, int8_t type);
PLAIN_API void unlock(mutex_t &mutex, int8_t type);

template <typename T>
class unique_lock : noncopyable {

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

};

struct PLAIN_API dataheader_struct {
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
   bool init(uint32_t key, size_t size, uint8_t type = Smpt::Default);
   void release();
   T *new_obj();
   T *get_obj(int32_t index);
   void delete_obj(T *obj);
   size_t size() const { return size_; };
   uint32_t key() const { return key_; };
   bool dump(const char *filename);
   bool merge(const char *filename);
   bool full() const { return size_ - 1 == (size_t)get_position(); };
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

  group_item_struct()
    : index{0}, size{0}, position{0}, header_size{0}, data_size{0},
    same_header{true}, name{""} {}
};

struct group_header_struct {

  uint8_t flag;

  group_header_struct() : flag{0} {}
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
    mutex.exchange(std::to_underlying(Flag::Free));
    version = 0;
    status = 0;
  }

  group_item_header_struct()
    : pool_position{0}, mutex{std::to_underlying(Flag::Free)}, version{0},
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
   std::unordered_map<int16_t, group_item_t> group_conf_;
   std::unique_ptr<Base> ref_obj_pointer_;
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

template <typename T>
inline void data_template<T>::set_usestatus(int8_t status, int8_t flag) {
  unique_lock<dataheader_t> auto_lock(header, flag);
  header.usestatus = status;
}

template <typename T>
inline int8_t data_template<T>::get_usestatus(int8_t flag) {
  unique_lock<dataheader_t> auto_lock(header, flag);
  return header.usestatus;
}

template <typename T>
data_template<T> &data_template<T>::operator = (const data_template &object) {
  header = object.header;
  data = object.data;
  return *this;
}

template <typename T>
data_template<T> *data_template<T>::operator = (const data_template *object) {
  if (object) {
    header = object->header;
    data = object->data;
  }
  return this;
}

template <typename T>
inline UnitPool<T>::UnitPool() :
  objs_(nullptr),
  ref_obj_pointer_(nullptr),
  size_(0),
  key_(0),
  data_extend_size_(0),
  header_extend_size_{0},
  ready_(false) {
  //do nothing.
}

template <typename T>
inline UnitPool<T>::~UnitPool() {
  safe_delete_array(objs_);
}

template <typename T>
bool UnitPool<T>::init(uint32_t _key, size_t _size, uint8_t type) {
  if (ready_) return true;
  std::unique_ptr<Base> p{new Base()};
  ref_obj_pointer_ = std::move(p);
  Assert(ref_obj_pointer_);
  if (is_null(ref_obj_pointer_)) return false;
  bool result = true;
  bool need_init = false;
  auto full_size = sizeof(header_t) + 
                   header_extend_size_ + 
                   (sizeof(T) + data_extend_size_ ) * _size;
  result = ref_obj_pointer_->attach(_key, full_size, false);
  if (std::to_underlying(Smpt::Default) == type && !result) {
    result = ref_obj_pointer_->create(_key, full_size);
    need_init = true;
  } else if (!result) {
    return false;
  }
  if (!result && CmdMode::ClearAll == GLOBALS["app.cmdmodel"]) {
    return true;
  } else if (!result) {
    LOG_ERROR << "init failed";
    Assert(result);
    return false;
  }
  size_ = _size;
  set_position(0);
  objs_ = new T *[size_];
  if (is_null(objs_)) return false;
  for (decltype(size_) i = 0; i < size_; ++i) {
    auto objsize = static_cast<uint32_t>(sizeof(T) + data_extend_size_);
    char *data = ref_obj_pointer_->get(objsize, i);
    if (data_extend_size_ > 0 && need_init) {
      memset(&data[sizeof(T)], 0, data_extend_size_);
    }
    objs_[i] = reinterpret_cast<T *>(data);
    objs_[i]->set_extend_size(data_extend_size_);
    if (is_null(objs_[i])) {
      Assert(objs_[i]);
      return false;
    }
    if (need_init) objs_[i]->init();
  }
  key_ = _key;
  ready_ = true;
  return true;
}

template <typename T>
inline void UnitPool<T>::release() {
  if (!is_null(ref_obj_pointer_)) ref_obj_pointer_->release();
}

template <typename T>
T *UnitPool<T>::new_obj() {
  Assert(ref_obj_pointer_);
  header_t *header = ref_obj_pointer_->header();
  Assert(header);
  unique_lock<header_t> auto_lock(*header, std::to_underlying(Flag::MixedWrite));
  if (header->pool_position >= size_) return nullptr;
  T *obj = objs_[header->pool_position];
  obj->set_pool_id(static_cast<int32_t>(header->pool_position));
  ++(header->pool_position);
  obj->set_key(key_);
  return obj;
}

template <typename T>
void UnitPool<T>::delete_obj(T *obj) {
  Assert(objs_ != nullptr && ref_obj_pointer_ != nullptr);
  header_t *header = ref_obj_pointer_->header();
  Assert(header);
  unique_lock<header_t> auto_lock(*header, std::to_underlying(Flag::MixedWrite));
  Assert(header->pool_position > 0);
  if (is_null(obj) || header->pool_position <= 0) return;
  auto delete_index = static_cast<uint32_t>(obj->get_pool_id());
  Assert(delete_index < header->pool_position);
  --(header->pool_position);
  if (0 == header->pool_position) return;
  T *_delete_obj = objs_[delete_index];
  T *swap_obj = objs_[header->pool_position];
  char *pointer = reinterpret_cast<char *>(_delete_obj);
  char *swappointer = reinterpret_cast<char *>(swap_obj);
  auto data_size = sizeof(T) + data_extend_size_;
  memcpy(pointer, swappointer, data_size);
  objs_[header->pool_position]->set_pool_id(ID_INVALID);
}

template <typename T>
inline T *UnitPool<T>::get_obj(int32_t index) {
  return index >= 0 && index < (int32_t)size_ ? objs_[index] : nullptr;
}

template <typename T>
inline int32_t UnitPool<T>::get_position() const { 
  if (is_null(ref_obj_pointer_)) return -1;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, std::to_underlying(Flag::MixedRead));
  return header->pool_position;
}

template <typename T>
inline void UnitPool<T>::set_position(int32_t position) { 
  if (is_null(ref_obj_pointer_)) return;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, std::to_underlying(Flag::MixedWrite));
  header->pool_position = position;
}

template <typename T>
inline bool UnitPool<T>::dump(const char *filename) {
  if (is_null(ref_obj_pointer_)) return false;
  return ref_obj_pointer_->dump(filename);
}

template <typename T>
inline bool UnitPool<T>::merge(const char *filename) {
  if (is_null(ref_obj_pointer_)) return false;
  return ref_obj_pointer_->merge(filename);
}

template <typename T>
inline void UnitPool<T>::set_version(uint32_t version) {
  if (is_null(ref_obj_pointer_)) return;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, std::to_underlying(Flag::MixedWrite));
  header->version = version;
}

template <typename T>
inline uint32_t UnitPool<T>::get_version() const {
  if (is_null(ref_obj_pointer_)) return 0;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, std::to_underlying(Flag::MixedWrite));
  return header->version;
}

template <typename T>
inline header_t *UnitPool<T>::get_header() {
  if (is_null(ref_obj_pointer_)) return nullptr;
  return ref_obj_pointer_->header();
}

template <typename T>
inline Node<T>::Node() :
  ready_(false),
  read_flag_(Flag::Free),
  write_flag_(Flag::Free),
  pool_(nullptr),
  recover_(false) {
  //do nothing.
}

template <typename T>
inline Node<T>::~Node() {
}

template <typename T>
inline void Node<T>::clear() {
  ready_ = false;
  read_flag_ = Flag::Free;
  write_flag_ = Flag::Free;
  recover_ = false;
}

template <typename T>
bool Node<T>::init(uint32_t _key, size_t _size) {
  if (ready_) return true;
  pool_ = new UnitPool<T>;
  if (is_null(pool_)) return false;
  if (!pool_->init(_key, _size)) return false;
  pool_->set_head_version(0);
  ready_ = init_after();
  return ready_;
}

template <typename T>
bool Node<T>::init_after() {
  if (CmdMode::ClearAll == GLOBALS["app.cmdmodel"]) return true;
  auto pool_size = size();
  if (recover_) {
    for (decltype(pool_size) i = 0; i < pool_size; ++i) {
      T *object = pool_->get_obj(i);
      if (is_null(object)) {
        Assert(object);
        return false;
      }
      auto usestatus = object->get_usestatus(write_flag_);
      if (Use::FreeEx == usestatus)
        object->set_usestatus(Use::ReadyFree, write_flag_);
    }
    LOG_INFO << "recover";
    return true;
  }
  for (decltype(pool_size) i = 0; i < pool_size; ++i) {
    T *object = pool_->get_obj(i);
    if (is_null(object)) {
      Assert(object); 
      return false;
    }
    object->header_clear();
    object->set_usestatus(Use::Free, write_flag_);
  }
  return true;
}

template <typename T>
bool Node<T>::empty() const {
  if (is_null(pool_)) return true;
  auto pool_max_size = pool_->max_size();
  for (decltype(pool_max_size) i = 0; i < pool_max_size; ++i) {
    T *object = pool_->get_obj(i);
    if (is_null(object)) {
      Assert(object);
      return false;
    }
    object->clear();
    auto usestatus = object->get_usestatus(write_flag_);
    object->set_usestatus(Use::Free, write_flag_);
  }
  return true;
}

template <typename T>
inline bool Node<T>::full() const {
  bool result = true;
  if (!is_null(pool_)) result = pool_->full();
  return result;
}

template <typename T>
inline size_t Node<T>::size() const {
  size_t _size = 0;
  if (!is_null(pool_)) _size = pool_->get_position();
  return size;
}

template <typename T>
inline size_t Node<T>::max_size() const {
  size_t _size = 0;
  if (!is_null(pool_)) _size = pool_->max_size();
  return _size;
}

template <typename T>
size_t Node<T>::hold_count() const {
  if (is_null(pool_)) return 0;
  auto pool_size = size();
  size_t result = 0;
  for (decltype(pool_size) i = 0; i < pool_size; ++i) {
    T *object = pool_->get_obj(i);
    if (is_null(object)) {
      Assert(object);
      return 0;
    }
    auto usestatus = object->single_usestatus();
    if (Use::HoldData == usestatus ||
        Use::ReadyFree == usestatus ||
        Use::FreeEx == usestatus) ++result;
  }
  return result;
}

template <typename T>  
template <class F, class... Args>
auto Node<T>::handle(F&& f, Args&&... args) 
  -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;
  auto task = std::make_shared< std::packaged_task<return_type()> >(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
  std::future<return_type> res = task->get_future();
  *task();
  return res;
}

template <typename T>
void Node<T>::set_minutes(uint32_t index, uint32_t minutes) {
  if (is_null(pool_)) return;
  uint32_t poolsize_max = pool_->get_max_size();
  if (index >= poolsize_max) return;
  T *object = pool_->get_obj(index);
  if (!is_null(object)) {
    object->setminutes(minutes, write_flag_);
  }
}

inline void clear([[maybe_unused]] int32_t key) {
#if OS_UNIX
  char cmd[256]{0}; char result[512]{0};
  snprintf(cmd, sizeof(cmd) - 1, "ipcrm -M %u\n", key);
  plain::exec(cmd, result, sizeof(result) - 1);
#endif
}

} // namespace plain::memory::share

#ifndef share_lock
#define share_lock(t,o,n,f) plain_sys::memory::share::unique_lock<t> n(o, f)
#endif

#endif //PLAIN_SYS_MEMORY_SHARE_H_
