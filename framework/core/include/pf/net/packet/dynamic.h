/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id dynamic.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2015/04/08 15:56
 * @uses net dynamic packet class
 *       cn: 这个是动态的网络包，数据不规则，可以使用在任何地方
 *           但强烈建议不要使用在C++中，可以使用在脚本内，动态的包虽然方便，
 *           但是不安全，也同时存在性能上的消耗
 *
 *       test time: 2015-4-9 18:26:41
 */
#ifndef PF_NET_PACKET_DYNAMIC_H_
#define PF_NET_PACKET_DYNAMIC_H_

#include "pf/net/packet/config.h"
#include "pf/net/packet/interface.h"
#include "pf/sys/memory/dynamic_allocator.h"

namespace pf_net {

namespace packet {

class PF_API Dynamic : public packet::Interface {

 public:
   Dynamic();
   Dynamic(uint16_t id); //如果使用这个构造，默认为可以写的包
   virtual ~Dynamic();

 public:
   void write(const char *buffer, uint32_t length);
   uint32_t read(char *buffer, uint32_t length);
   void set_readable(bool flag) { readable_ = flag; };
   void set_writeable(bool flag) { writeable_ = flag; };
   void clear();

 public:
   virtual void set_id(uint16_t id) { id_ = id; }
   virtual void set_size(uint32_t _size) { size_ = _size; }

 public:
   virtual bool read(stream::Input &istream);
   virtual bool write(stream::Output &ostream);
   virtual uint16_t get_id() const { return id_; }
   virtual uint32_t size() const { return size_; }


 public: //write_*常用方法
   void write_int8(int8_t value);
   void write_uint8(uint8_t value);
   void write_int16(int16_t value);
   void write_uint16(uint16_t value);
   void write_int32(int32_t value);
   void write_uint32(uint32_t value);
   void write_int64(int64_t value);
   void write_uint64(uint64_t value);
   void write_string(const char *value);
   void write_float(float value);
   void write_double(double value);
   void write_bytes(const unsigned char *value, size_t size);
   void write_raw(const char *value, size_t size);

 public:
   int8_t read_int8();
   uint8_t read_uint8();
   int16_t read_int16();
   uint16_t read_uint16();
   int32_t read_int32();
   uint32_t read_uint32();
   int64_t read_int64();
   uint64_t read_uint64();
   void read_string(char *buffer, size_t size);
   float read_float();
   double read_double();
   uint32_t read_bytes(unsigned char *value, size_t size);
   void read_raw(char *value, size_t size);

 public:
   //some useful.
   Dynamic &operator >> (bool &var) {
     var = 1 == read_int8() ? true : false;
     return *this;
   };
   Dynamic &operator >> (int8_t &var) {
     var = read_int8();
     return *this;
   };
   Dynamic &operator >> (uint8_t &var) {
     var = read_uint8();
     return *this;
   };
   Dynamic &operator >> (int16_t &var) {
     var = read_int16();
     return *this;
   };
   Dynamic &operator >> (uint16_t &var) {
     var = read_uint16();
     return *this;
   };
   Dynamic &operator >> (int32_t &var) {
     var = read_int32();
     return *this;
   };
   Dynamic &operator >> (uint32_t &var) {
     var = read_uint32();
     return *this;
   };
   Dynamic &operator >> (int64_t &var) {
     var = read_int64();
     return *this;
   };
   Dynamic &operator >> (uint64_t &var) {
     var = read_uint64();
     return *this;
   };
   Dynamic &operator >> (char *&var) { //Not safe.
     uint32_t _size = read_uint32();
     read_string(var, _size);
     return *this;
   };
   Dynamic &operator >> (std::string &var) { //Not safe.
     uint32_t _size = read_uint32();
     char temp[2048]{0,};
     read_string(temp, _size);
     var = temp;
     return *this;
   };

 public:
   Dynamic &operator << (bool var) {
     write_int8(var ? 1 : 0);
     return *this;
   };
   Dynamic &operator << (int8_t var) {
     write_int8(var);
     return *this;
   };
   Dynamic &operator << (uint8_t var) {
     write_uint8(var);
     return *this;
   };
   Dynamic &operator << (int16_t var) {
     write_int16(var);
     return *this;
   };
   Dynamic &operator << (uint16_t var) {
     write_uint16(var);
     return *this;
   };
   Dynamic &operator << (int32_t var) {
     write_int32(var);
     return *this;
   };
   Dynamic &operator << (uint32_t var) {
     write_uint32(var);
     return *this;
   };
   Dynamic &operator << (int64_t var) {
     write_int64(var);
     return *this;
   };
   Dynamic &operator << (uint64_t var) {
     write_uint64(var);
     return *this;
   };
   Dynamic &operator << (const char *var) {
     write_string(var);
     return *this;
   };
   Dynamic &operator << (const std::string &var) {
     write_string(var.c_str());
     return *this;
   };

 protected:
   void check_memory(uint32_t length);

 private:
   uint16_t id_; //包ID
   pf_sys::memory::DynamicAllocator allocator_; //内存分配
   uint32_t offset_; //动态包写入或读取到的位置
   uint32_t size_; //包的大小
   bool readable_; //是否可读
   bool writeable_; //是否可写

};

} //namespace packet

} //namespace pf_net

#endif //PF_NET_PACKET_DYNAMIC_H_
