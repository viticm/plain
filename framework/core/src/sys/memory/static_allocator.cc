#include "pf/basic/logger.h"
#include "pf/sys/memory/static_allocator.h"

namespace pf_sys {

namespace memory {

StaticAllocator::StaticAllocator() :
  buffer_{nullptr},
  size_{0},
  offset_{0} {
}

StaticAllocator::~StaticAllocator() {
  //do nothing
}

void StaticAllocator::init(char *buffer, size_t _size) {
  buffer_ = buffer;
  size_ = _size;
}

void *StaticAllocator::malloc(size_t _size) {
  using namespace pf_basic;
  if (offset_ + _size > size_) {
    SLOW_ERRORLOG("error",
                  "[sys.memory] (StaticAllocator::malloc)"
                  " out of memory allocating %d bytes",
                  _size);
    Assert(false);
    return nullptr;
  }
  char *pointer = &buffer_[offset_]; 
  offset_ += _size;
  return reinterpret_cast<void *>(pointer);
}

void *StaticAllocator::calloc(size_t _size, size_t count) {
  void *pointer = malloc(count * _size);
  memset(pointer, 0, count * _size);
  return reinterpret_cast<void *>(pointer);
}

void *StaticAllocator::realloc(void *data, size_t newsize) {
  auto _data = static_cast<char *>(data);
  auto buffer = static_cast<char *>(buffer_);
  Assert(data >= buffer_ && data < buffer_ + size_);
  auto data_offset = _data - buffer;
  if (static_cast<size_t>(data_offset) != offset_) {
   SLOW_ERRORLOG("error",
                  "[sys.memory] (DynamicAllocator::realloc)"
                  " the realloc pointer not on top");
    Assert(false);
    return nullptr;
  }
  size_t size_ofdata = offset_ - static_cast<size_t>(_data - buffer);
  size_t _size = newsize - size_ofdata;
  if (offset_ + _size > size_) {
    SLOW_ERRORLOG("error",
                  "[sys.memory] (StaticAllocator::malloc)"
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

void StaticAllocator::free(void *) {
  //Assert(data >= buffer_ && data < buffer_ + size_);
}

}; //namespace memory

}; //namespace pf_sys
