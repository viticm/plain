/**
 * PAP Engine ( https://github.com/viticm/pap )
 * $Id share.tcc
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2015/04/10 10:56
 * @uses system share memory template class implement
 */
#ifndef PF_SYS_MEMORY_SHARE_TCC_
#define PF_SYS_MEMORY_SHARE_TCC_

#include "pf/sys/memory/config.h"
#include "pf/basic/time_manager.h"
#include "pf/sys/memory/share.h"
#include "pf/sys/util.h"

namespace pf_sys {

namespace memory {

namespace share {

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
bool UnitPool<T>::init(uint32_t key, size_t size, uint8_t type) {
  if (ready_) return true;
  std::unique_ptr<Base> p{new Base()};
  ref_obj_pointer_ = std::move(p);
  Assert(ref_obj_pointer_);
  if (is_null(ref_obj_pointer_)) return false;
  bool result = true;
  bool need_init = false;
  auto full_size = sizeof(header_t) + 
                   header_extend_size_ + 
                   (sizeof(T) + data_extend_size_ ) * size;
  result = ref_obj_pointer_->attach(key, full_size, false);
  if (kSmptDefault == type && !result) {
    result = ref_obj_pointer_->create(key, full_size);
    need_init = true;
  } else if (!result) {
    return false;
  }
  if (!result && GLOBALS["app.cmdmodel"] == kCmdModelClearAll) {
    return true;
  } else if (!result) {
    SLOW_ERRORLOG(
        "sharememory",
        "[sys.memory.share] (UnitPool::init) failed");
    Assert(result);
    return false;
  }
  size_ = size;
  set_position(0);
  objs_ = new T * [size_];
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
  key_ = key;
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
  unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
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
  unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
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
  return index >= 0 && index < size_ ? objs_[index] : nullptr;
}

template <typename T>
inline int32_t UnitPool<T>::get_position() const { 
  if (is_null(ref_obj_pointer_)) return -1;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, kFlagMixedRead);
  return header->pool_position;
}

template <typename T>
inline void UnitPool<T>::set_position(int32_t position) { 
  if (is_null(ref_obj_pointer_)) return;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
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
  unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
  header->version = version;
}

template <typename T>
inline uint32_t UnitPool<T>::get_version() const {
  if (is_null(ref_obj_pointer_)) return 0;
  header_t *header = ref_obj_pointer_->header();
  unique_lock<header_t> auto_lock(*header, kFlagMixedWrite);
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
  read_flag_(kFlagFree),
  write_flag_(kFlagFree),
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
  read_flag_ = kFlagFree;
  write_flag_ = kFlagFree;
  recover_ = false;
}

template <typename T>
bool Node<T>::init(uint32_t key, size_t size) {
  if (ready_) return true;
  pool_ = new UnitPool<T>;
  if (is_null(pool_)) return false;
  if (!pool_->init(key, size)) return false;
  pool_->set_head_version(0);
  ready_ = init_after();
  return ready_;
}

template <typename T>
bool Node<T>::init_after() {
  if (GLOBALS["app.cmdmodel"] == kCmdModelClearAll) return true;
  auto pool_size = size();
  if (recover_) {
    for (decltype(pool_size) i = 0; i < pool_size; ++i) {
      T *object = pool_->get_obj(i);
      if (is_null(object)) {
        Assert(object);
        return false;
      }
      auto usestatus = object->get_usestatus(write_flag_);
      if (kUseFreeEx == usestatus)
        object->set_usestatus(kUseReadyFree, write_flag_);
    }
    SLOW_LOG(GLOBALS["app.name"].c_str(),
              "[sys.memory.share] Node<T>::initafter recover"); 
    return true;
  }
  for (decltype(pool_size) i = 0; i < pool_size; ++i) {
    T *object = pool_->get_obj(i);
    if (is_null(object)) {
      Assert(object); 
      return false;
    }
    object->header_clear();
    object->set_usestatus(kUseFree, write_flag_);
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
    object->set_usestatus(kUseFree, write_flag_);
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
  size_t size = 0;
  if (!is_null(pool_)) size = pool_->get_position();
  return size;
}

template <typename T>
inline size_t Node<T>::max_size() const {
  size_t size = 0;
  if (!is_null(pool_)) size = pool_->max_size();
  return size;
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
    if (kUseHoldData == usestatus ||
        kUseReadyFree == usestatus ||
        kUseFreeEx == usestatus) ++result;
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

inline void clear(int32_t key) {
#if OS_UNIX
  char cmd[256]{0}; char result[512]{0};
  snprintf(cmd, sizeof(cmd) - 1, "ipcrm -M %u\n", key);
  pf_sys::util::exec(cmd, result, sizeof(result) - 1);
#else
  UNUSED(key);
#endif
}

}; //namespace share

}; //namespace memory

}; //namespace pf_sys

#endif //PF_SYS_MEMORY_SHARE_TCC_
