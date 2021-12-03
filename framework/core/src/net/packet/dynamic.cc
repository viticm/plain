#include "pf/basic/endian.h"
#include "pf/basic/logger.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/packet/dynamic.h"

namespace pf_net {

namespace packet {

Dynamic::Dynamic() :
  id_{0},
  offset_{0},
  size_{0},
  readable_{false},
  writeable_{false} {
  allocator_.malloc(2048);
}


Dynamic::Dynamic(uint16_t id) :
  id_{id},
  offset_{0},
  size_{0},
  readable_{false},
  writeable_{true} {
  allocator_.malloc(2048);
}

Dynamic::~Dynamic() {
  //do nothing
}

void Dynamic::clear() {
  allocator_.malloc(2048);
  offset_ = 0;
  size_ = 0;
  readable_ = false;
  writeable_ = false;
  id_ = 0;
}

void Dynamic::write(const char *buffer, uint32_t length) {
  if (!writeable_) return;
  //(length - (allocator_.getsize() - offset_)) + allocator_.getsize();
  uint32_t checklength = length + offset_;
  check_memory(checklength);
  char *_buffer = reinterpret_cast<char *>(allocator_.get()) + offset_;
  memcpy(_buffer, buffer, length);
  offset_ += length;
  size_ += length;
}

uint32_t Dynamic::read(char *buffer, uint32_t length) {
  if (!readable_) return 0;
  if (0 == size_ || offset_ + length > size_) return 0;
  char *_buffer = reinterpret_cast<char *>(allocator_.get()) + offset_;
  memcpy(buffer, _buffer, length);
  offset_ += length;
  return length;
}

void Dynamic::write_int8(int8_t value) {
  write((char *)&value, sizeof(value));
}
   
void Dynamic::write_uint8(uint8_t value) {
  write((char *)&value, sizeof(value));
}
   
void Dynamic::write_int16(int16_t value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
  
void Dynamic::write_uint16(uint16_t value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
   
void Dynamic::write_int32(int32_t value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
   
void Dynamic::write_uint32(uint32_t value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
   
void Dynamic::write_int64(int64_t value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
   
void Dynamic::write_uint64(uint64_t value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
   
void Dynamic::write_string(const char *value) {
  uint32_t length = static_cast<uint32_t>(strlen(value));
  write_uint32(length);
  write(value, length);
}
   
void Dynamic::write_float(float value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}
   
void Dynamic::write_double(double value) {
  write((char *)&(value = PF_HTON(value)), sizeof(value));
}

void Dynamic::write_bytes(const unsigned char *value, size_t _size) {
  write_uint32(_size);
  write((const char *)value, _size);
}

int8_t Dynamic::read_int8() {
  int8_t result = 0;
  read((char *)&result, sizeof(result));
  return result;
}
   
uint8_t Dynamic::read_uint8() {
  uint8_t result = 0;
  read((char *)&result, sizeof(result));
  return result;
}

int16_t Dynamic::read_int16() {
  int16_t result = 0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}
   
uint16_t Dynamic::read_uint16() {
  uint16_t result = 0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}

int32_t Dynamic::read_int32() {
  int32_t result = 0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}
   
uint32_t Dynamic::read_uint32() {
  uint32_t result = 0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}
   
int64_t Dynamic::read_int64() {
  int64_t result = 0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}
   
uint64_t Dynamic::read_uint64() {
  uint64_t result = 0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}
   
void Dynamic::read_string(char *buffer, size_t _size) {
  uint32_t length = read_uint32();
  if (length <= 0 || _size < length) return;
  read(buffer, length);
}

float Dynamic::read_float() {
  float result = .0f;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}
   
double Dynamic::read_double() {
  double result = .0;
  read((char *)&result, sizeof(result));
  return PF_NTOH(result);
}

uint32_t Dynamic::read_bytes(unsigned char *value, size_t _size) {
  auto length = read_uint32();
  return read((char *)value, _size > length ? length : _size);
}

void Dynamic::check_memory(uint32_t length) {
  if (length > NET_PACKET_DYNAMIC_SIZEMAX) {
    SLOW_ERRORLOG(NET_MODULENAME,
                  "[net.packet] (Dynamic::checkmemory) length out limit"
                  " (%d, %d)",
                  length,
                  NET_PACKET_DYNAMIC_SIZEMAX);
    return;
  }
  while (length >= allocator_.size()) {
    //注意这里的length是指要写入完整数据拥有的最小的长度
    allocator_.malloc(allocator_.size() + NET_PACKET_DYNAMIC_ONCESIZE);
  }
}

bool Dynamic::read(stream::Input &istream) {
  check_memory(size_);
  char *_buffer = reinterpret_cast<char *>(allocator_.get());
  memset(_buffer, 0, allocator_.size());
  uint32_t _size = istream.read(_buffer, size_);
  bool result = _size == size_;
  return result;
}

bool Dynamic::write(stream::Output &ostream) {
  //DEBUGPRINTF("Dynamic::write size: %d", size_);
  if (size_ <= 0 || 0 == id_) return false;
  char *_buffer = reinterpret_cast<char *>(allocator_.get());
  uint32_t _size = ostream.write(_buffer, size_);
  bool result = _size == size_;
  return result;
}

} //namespace packet

} //namespace pf_net
