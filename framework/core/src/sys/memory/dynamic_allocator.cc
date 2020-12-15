#include "pf/basic/logger.h"
#include "pf/sys/memory/dynamic_allocator.h"

namespace pf_sys {

namespace memory {

DynamicAllocator::DynamicAllocator():
  pointer_{nullptr},
  size_{0},
  offset_{0} {
}

DynamicAllocator::~DynamicAllocator() {
  free();
}

void *DynamicAllocator::malloc(size_t _size) {
  if (size_ == _size) return pointer_;
  void *pointer = reinterpret_cast<void *>(new char[_size]);
  if (is_null(pointer)) {
    Assert(false);
    return nullptr;
  }
  memset(pointer, 0, _size);
  if (!is_null(pointer_)) {
    size_t copysize = _size > size_ ? size_ : _size;
    memcpy(pointer, pointer_, copysize);
    free();
  }
  pointer_ = pointer;
  size_ = _size;
  return pointer_;
}

void *DynamicAllocator::calloc(size_t _size, size_t count) {
  auto leftsize = size_ - offset_;
  auto needsize = count * _size;
  if (needsize > leftsize) {
    if (!malloc(needsize - leftsize + size_ + 1)) return nullptr;
  }
  auto pointer = cast(char *, pointer_) + offset_;
  memset(pointer, 0, needsize);
  offset_ += needsize;
  return reinterpret_cast<void *>(pointer);
}

void *DynamicAllocator::realloc(void *data, size_t newsize) {
  auto pointer = static_cast<char *>(pointer_);
  auto _data = static_cast<char *>(data);
  Assert(_data >= pointer && _data < pointer + size_);
  auto data_offset = _data - pointer;
  if (static_cast<size_t>(data_offset) != offset_) {
   SLOW_ERRORLOG("error",
                  "[sys.memory] (DynamicAllocator::realloc)"
                  " the realloc pointer not on top");
    Assert(false);
    return nullptr;
  }
  size_t size_ofdata = offset_ - static_cast<size_t>(_data - pointer);
  size_t _size = newsize - size_ofdata;
  if (offset_ + _size > size_) {
    SLOW_ERRORLOG("error",
                  "[sys.memory] (DynamicAllocator::malloc)"
                  " out of memory allocating %d bytes",
                  _size);
    Assert(false);
    return nullptr;
  } else {
    offset_ += _size;
    return data;
  }
  return nullptr;
}

void DynamicAllocator::free() {
  char *pointer = reinterpret_cast<char*>(pointer_);
  if (!is_null(pointer)) {
    delete[] pointer;
    pointer_ = nullptr;
  }
  size_ = 0;
}

void *DynamicAllocator::get() {
  return pointer_;
}

} //namespace memory

} //namespace pf_sys
