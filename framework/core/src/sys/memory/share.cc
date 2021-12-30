#include "pf/basic/logger.h"
#include "pf/basic/util.h"
#if OS_UNIX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#elif OS_WIN
#include <winbase.h>
#endif
#include "pf/sys/memory/share.h"
#include "pf/basic/io.tcc"

namespace pf_sys {

namespace memory { 

namespace share {

header_struct::header_struct() :
  key{0}, 
  size{0},
  version{0},
  pool_position{0},
  mutex{kFlagFree} {

}                                                                                  

void header_struct::clear() {
  key = 0;                                                                       
  size = 0;                                                                      
  mutex.exchange(kFlagFree);
  version = 0;                                                                   
  pool_position = 0;
}

void header_struct::lock(int8_t type) {
  share::lock(mutex, type);
}
  
void header_struct::unlock(int8_t type) {
  share::unlock(mutex, type);
}
                                                                                  
header_struct::~header_struct() {                                          
  //do nothing
}

dataheader_struct::dataheader_struct() {
  clear();
}

dataheader_struct::~dataheader_struct() {
  //do nothing
}

void dataheader_struct::clear() {
  key = 0;
  version = 0;
  usestatus = kUseFree;
  pool_id = ID_INVALID;
  mutex.exchange(kFlagFree);
}

  
dataheader_t &dataheader_struct::operator = (const dataheader_t &object) {
  key = object.key;
  version = object.version;
  int8_t flag = object.mutex;
  mutex.exchange(flag);
  pool_id = object.pool_id;
  return *this;
}
  
dataheader_t *dataheader_struct::operator = (const dataheader_t *object) {
  if (object) {
    key = object->key;
    version = object->version;
    int8_t flag = object->mutex;
    mutex.exchange(flag);
    pool_id = object->pool_id;

  }
  return this;
}
 
//struct end --

namespace api {

#if OS_UNIX
int32_t create(uint32_t key, size_t size) {
  int32_t handle = 0;
#elif OS_WIN
HANDLE create(uint32_t key, size_t size) {
    HANDLE handle = nullptr;
#endif
#if OS_UNIX
    handle = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
    if (HANDLE_INVALID == handle) {
      SLOW_ERRORLOG(
          GLOBALS["app.name"].c_str(),
          "[sys.memory.share] (api::create) handle = %d," 
          " key = %d, error: %d",
          handle, 
          key, 
          errno);
    }
#elif OS_WIN
    char buffer[65]{0,};
    snprintf(buffer, sizeof(buffer) - 1, "%d", key);
    handle = (CreateFileMapping(reinterpret_cast<HANDLE>(0xFFFFFFFFFFFFFFFF), 
                                nullptr, 
                                PAGE_READWRITE, 
                                0, 
                                (DWORD)size, 
                                buffer));
#endif
    return handle;
}
#if OS_UNIX
int32_t open(uint32_t key, size_t size, bool errorlog) {
  int32_t handle = 0;
#elif OS_WIN
HANDLE open(uint32_t key, size_t, bool) {
  HANDLE handle = nullptr;
#endif
#if OS_UNIX
    handle = shmget(key, size, 0);
    if (HANDLE_INVALID == handle && errorlog) {
      SLOW_ERRORLOG(
          GLOBALS["app.name"].c_str(), 
          "[sys.memory.share] (api::open) handle = %d,"
          " key = %d, error: %d", 
          handle, 
          key, 
          errno);
    }
#elif OS_WIN
    char buffer[65];
    memset(buffer, '\0', sizeof(buffer));
    snprintf(buffer, sizeof(buffer) - 1, "%d", key);
    handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, true, (LPCWSTR)buffer);
#endif
    return handle;
}

#if OS_UNIX
char *map(int32_t handle) {
#elif OS_WIN
char *map(HANDLE handle) {
#endif
    char *result;
#if OS_UNIX
    result = static_cast<char *>(shmat(handle, 0, 0));
#elif OS_WIN
    result = 
      static_cast<char *>(MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0));
#endif
    return result;
}

void unmap(char *pointer) {
#if OS_UNIX
    shmdt(pointer);
#elif OS_WIN
    UnmapViewOfFile(pointer);
#endif
}

#if OS_UNIX
void close(int32_t handle) {
#elif OS_WIN
void close(HANDLE handle) {
#endif
#if OS_UNIX
    shmctl(handle, IPC_RMID, 0);
#elif OS_WIN
    CloseHandle(reinterpret_cast<HANDLE>(handle));
#endif
}

} //namespace api


//-- class start
Base::Base() :
  size_{0},
  data_{nullptr},
  header_{nullptr},
  handle_{HANDLE_INVALID} {
}

Base::~Base() {
  //do nothing
}

bool Base::create(uint32_t _key, size_t _size) {
    if (GLOBALS["app.cmdmodel"] == kCmdModelClearAll) return false;
    handle_ = api::create(_key, _size);
    if (HANDLE_INVALID == handle_) {
      SLOW_ERRORLOG(
          GLOBALS["app.name"].c_str(), 
          "[sys.memory.share] (Base::create)"
          " failed! handle = %d, key = %d",
          handle_, 
          _key);
      return false;
    }
    header_ = api::map(handle_);
    if (header_) {
      data_ = header_ + sizeof(header_t);
      header()->clear();
      header()->key = _key;
      header()->size = _size;
      size_ = _size;
      SLOW_LOG(
          GLOBALS["app.name"].c_str(), 
          "[sys.memory.share] (Base::create)"
          " success! handle = %d, key = %d",
          handle_, 
          _key);
      return true;
    } else {
      SLOW_ERRORLOG(
          GLOBALS["app.name"].c_str(), 
          "[sys.memory.share] (Base::create)"
          "map failed! handle = %d, key = %d", 
          handle_, 
          _key);
      return false;
    }
    return false;
}

void Base::release() {
    if (header_) {
      api::unmap(header_);
      header_ = nullptr;
    }
    if (handle_) {
      api::close(handle_);
#if OS_UNIX
      handle_ = 0;
#elif OS_WIN
      handle_ = nullptr;
#endif
    }
    size_ = 0;
}

bool Base::attach(uint32_t _key, size_t _size, bool errorlog) {
    handle_ = api::open(_key, _size, errorlog);
    if (GLOBALS["app.cmdmodel"] == kCmdModelClearAll) {
      release();
      SLOW_LOG(
          GLOBALS["app.name"].c_str(),
          "[sys.memory.share] (Base::attach) close memory, key = %d", 
          _key);
      return false;
    }
    if (HANDLE_INVALID == handle_) {
      if (errorlog) {
        SLOW_ERRORLOG(
            GLOBALS["app.name"].c_str(), 
            "[sys.memory.share] (Base::attach) failed, key = %d", 
            _key); 
      }
      return false;
    }
    header_ = api::map(handle_);
    if (header_) {
      data_ = header_ + sizeof(header_t);
      Assert(header()->key == _key);
      Assert(header()->size == _size);
      size_ = _size;
      SLOW_LOG(
          GLOBALS["app.name"].c_str(), 
          "[sys.memory.share] (Base::attach) success, key = %d", 
          _key); 
      return true;
    } else {
      if (errorlog) {
        SLOW_ERRORLOG(
            GLOBALS["app.name"].c_str(), 
            "[sys.memory.share] (Base::attach) map failed, key = %d", 
            _key); 
      }
      return false;
    }
    return true;
}

char *Base::get(uint32_t index, size_t _size) {
    Assert(_size > 0);
    Assert(_size * index < size_);
    char *result;
    result = 
      (_size <= 0 || _size * index > size_) ? nullptr : data_ + _size * index;
    return result;
}

bool Base::dump(const char *filename) {
    Assert(filename);
    FILE* fp = fopen(filename, "wb");
    if (!fp) return false;
    fwrite(header_, 1, size_, fp);
    fclose(fp);
    return true;
}

bool Base::merge(const char *filename) {
    Assert(filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;
    fseek(fp, 0L, SEEK_END);
    int32_t filelength = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    fread(header_, filelength, 1, fp);
    fclose(fp);
    return true;
}

//class end --

//-- functions start

void lock(mutex_t &mutex, int8_t type) {
  if (GLOBALS["app.cmdmodel"] == kCmdModelRecover ||
      GLOBALS["app.status"] == kAppStatusStop) return;
  int32_t count = 0;
  int8_t flag{kFlagFree};
  while (!mutex.compare_exchange_weak(flag, type)) {
    if (GLOBALS["app.status"] == kAppStatusStop) break;
    flag = kFlagFree; ++count;
    std::this_thread::sleep_for(std::chrono::milliseconds(0));
    if (count > 100) {
#ifdef _DEBUG //Lock so many times, but normal in multi threads.
      char time_str[256] = {0};
      pf_basic::Logger::get_log_timestr(time_str, sizeof(time_str) - 1);
      pf_basic::io_cerr("%s[sys.memory] (share::lock) failed", time_str);
#endif
      count = 0;
    }
  }
}

void unlock(mutex_t &mutex, int8_t type) {
  if (GLOBALS["app.cmdmodel"] == kCmdModelRecover ||
      GLOBALS["app.status"] == kAppStatusStop) return;
  int8_t flag{type};
  int8_t count{0};
  while (!mutex.compare_exchange_weak(flag, kFlagFree)) {
    if (GLOBALS["app.status"] == kAppStatusStop) break;
    //auto cur = flag;
    flag = type; ++count;
    std::this_thread::sleep_for(std::chrono::milliseconds(0));
    if (count > 100) {
      //pf_basic::io_cdebug("cur: %d, type: %d", cur, type);
      mutex.exchange(type); //Not safe to unlock want.
#ifndef _DEBUG
      char time_str[256] = {0};
      pf_basic::Logger::get_log_timestr(time_str, sizeof(time_str) - 1);
      pf_basic::io_cerr("%s[sys.memory] (share::unlock) failed", time_str);
#endif
      count = 0;
    }
  }
}

GroupPool::GroupPool(uint32_t _key, const std::vector<group_item_t> &group) :
  key_{_key},
  size_{0},
  ready_{false}{
  size_ += sizeof(group_header_t);
  for (size_t i = 0; i < group.size(); ++i) {
    const group_item_t &item = group[i];
    group_item_t temp;
    temp.index = item.index;
    temp.position = static_cast<uint32_t>(size_);
    temp.size = item.size;
    temp.header_size = item.header_size;
    temp.data_size = item.data_size;
    temp.same_header = item.same_header;
    group_conf_[item.index] = temp;
    size_t _size{0};
    if (item.same_header) {
      _size = sizeof(group_item_header_t) + 
              item.header_size + 
              item.data_size * item.size;
    } else {
      _size = sizeof(group_item_header_t) +
              (item.header_size + item.data_size) * _size;
    }
    size_ += _size; 
  }
}

bool GroupPool::init(bool create) {
  if (0 == key_ || 0 == size_) return false;
  if (ready_) return true;
  std::unique_ptr<Base> ptr(new Base());
  ref_obj_pointer_ = std::move(ptr);
  Assert(ref_obj_pointer_);
  if (is_null(ref_obj_pointer_)) return false;
  bool result = true;
  bool need_init = false;
  result = ref_obj_pointer_->attach(key_, size_, false);
  if (create && !result) {
    result = ref_obj_pointer_->create(key_, size_);
    need_init = true;
  } else if (!result) {
    return false;
  }
  if (!result && GLOBALS["app.cmdmodel"] == kCmdModelClearAll) {
    return true;
  } else if (!result) {
    SLOW_ERRORLOG(
        "sharememory",
        "[sys][sharememory] (GroupPool::init) failed");
    Assert(result);
    return false;
  }

  if (need_init) {
    char *data = ref_obj_pointer_->get();
    memset(data, 0, size_);
    auto it = group_conf_.begin();
    auto it_end = group_conf_.end();
    for (;it != it_end; ++it) {
      auto _header = item_header(it->first);
      _header->clear();
    }
  }
  ready_ = true;
  return true;
}

char *GroupPool::get_data(int16_t index) {
  if (INDEX_INVALID == index || !is_valid_index(index)) return nullptr;
  const group_item_t &item = group_conf_[index];
  return ref_obj_pointer_->get() + sizeof(group_header_t) + item.position;
}

group_header_t *GroupPool::header() {
  return reinterpret_cast<group_header_t *>(ref_obj_pointer_->get());
}
   
char *GroupPool::item_data_header(int16_t index, int16_t data_index) {
  if (INDEX_INVALID == index || INDEX_INVALID == data_index) return nullptr;
  if (!is_valid_index(index)) return nullptr;
  char *data = get_data(index);
  const group_item_t &item = group_conf_[index];
  char *result = nullptr;
  size_t header_size = sizeof(group_item_header_t);
  if (static_cast<size_t>(data_index) >= item.size) return nullptr;
  if (item.same_header) {
    result = data + header_size;
  } else {
    result = 
      data + header_size + (item.header_size + item.data_size) * data_index;
  }
  return result;
}
  
group_item_header_t *GroupPool::item_header(int16_t index) {
  char *data = get_data(index);
  return reinterpret_cast<group_item_header_t *>(data);
}

int32_t GroupPool::item_data_postion(int16_t index, int16_t data_index) {
  if (!is_valid_index(index)) return INDEX_INVALID;
  int32_t position{INDEX_INVALID};
  const group_item_t &item = group_conf_[index];
  if (item.same_header) {
    position = static_cast<int32_t>(item.header_size + data_index * item.data_size + 
               sizeof(group_item_header_t));
  } else {
    position = static_cast<int32_t>(item.header_size + 
               (item.header_size + item.data_size) * data_index +
               sizeof(group_item_header_t));
  }
  return position;
}
   
char *GroupPool::item_data(int16_t index, int16_t data_index) {
  if (INDEX_INVALID == index || INDEX_INVALID == data_index) return nullptr;
  if (!is_valid_index(index)) return nullptr;
  char *data = get_data(index);
  const group_item_t &item = group_conf_[index];
  char *result = nullptr;
  auto position = item_data_postion(index, data_index);
  auto _size = item.data_size;
  if (!item.same_header) _size += item.header_size;
  auto fullsize = item.position + position + _size;
  Assert(fullsize <= size_);
  if (fullsize <= size_) {
    result = data + position;
  }
  return result;
}

void GroupPool::free() {
  for (size_t i = 0; i < group_conf_.size(); ++i) {
    const group_item_t &item = group_conf_[static_cast<int16_t>(i)];
    group_item_header_t *_header = item_header(item.index);
    unique_lock< group_item_header_t > auto_lock(*_header, kFlagMixedWrite);
    _header->pool_position = 0;
  }
}

char *GroupPool::alloc(int16_t index, int16_t &data_index) {
  group_item_header_t *_header = item_header(index);
  Assert(_header);
  unique_lock<group_item_header_t> auto_lock(*_header, kFlagMixedWrite);
  char *data = get_data(index);
  const group_item_t &item = group_conf_[index];
  char *result = nullptr;
  if (static_cast<size_t>(_header->pool_position) >= item.size) return nullptr;
  data_index = _header->pool_position;
  auto position = item_data_postion(index, data_index);
  auto _size = item.data_size;
  if (!item.same_header) _size += item.header_size;
  auto fullsize = item.position + position + _size;
  Assert(fullsize <= size_);
  if (fullsize <= size_) {
    result = data + position;
  }
  if (is_null(result)) return nullptr;
  memset(result, 0, _size);
  _header->pool_position += 1;
  return result;
}

int16_t GroupPool::free(int16_t index, int16_t data_index) {
  group_item_header_t *_header = item_header(index);
  Assert(_header);
  unique_lock<group_item_header_t> auto_lock(*_header, kFlagMixedWrite);
  const group_item_t &item = group_conf_[index];
  --(_header->pool_position);
  if (data_index >= _header->pool_position) return INDEX_INVALID;
  size_t header_size{0};
  if (!item.same_header) {
    header_size = item.header_size;
  }
  char *swap_data = item_data(index, _header->pool_position) - header_size;
  size_t data_size = header_size + item.data_size;
  char *delete_data = item_data(index, data_index) - header_size;
  memset(delete_data, 0, data_size);
  memcpy(delete_data, swap_data, data_size);
  return _header->pool_position;
}

//functions end --

} //namespace share

} //namespace share

} //namespace pf_sys
