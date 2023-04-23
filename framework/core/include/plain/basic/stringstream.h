/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id stringstream.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/01 17:22
 * @uses Like the <sstream>, 
 *       but this class will read/write by string buffer(char *) directly.
 *       * This use network(big) endian stream.
*/
#ifndef PLAIN_BASIC_STRINGSTREAM_H_
#define PLAIN_BASIC_STRINGSTREAM_H_

#include "plain/basic/config.h"
#include "plain/basic/endian.h"
#include "plain/sys/assert.h"

namespace plain {

class stringstream {

 public:
   stringstream(char *str, size_t size) :
     str_{str}, size_{size}, cur_{0} {}
   ~stringstream() {}

 public:
   void read(char *var, size_t size) {
     Assert(str_ != nullptr && size_ > 0);
     if (size + cur_ > size_) return;
     memcpy(var, str_ + cur_, size);
     cur_ += size;
   }
   void write(const char *var, size_t size) {
     Assert(str_ != nullptr && size_ > 0);
     if (size + cur_ > size_) return;
     memcpy(str_ + cur_, var, size);
     cur_ += size;
   }

 public:
   void clear() {
     cur_ = 0;
     memset(str_, 0, size_);
   }

   bool full() const {
     return cur_ >= size_;
   }

   size_t get_position() const {
     return cur_;
   }

   void set_position(size_t position) {
     Assert(position < size_);
     if (position >= size_) return;
     cur_ = position;
   }

 public:
   template <typename T>
   stringstream &operator>>(T &var) {
     read((char *)&var, sizeof(var));
     var = ntoh(var);
     return *this;
   }

   stringstream &operator>>(int8_t& var) {
     read((char *)&var, sizeof(var));
     return *this;
   }

   stringstream &operator>>(uint8_t& var) {
     read((char *)&var, sizeof(var));
     return *this;
   }

   stringstream &operator>>(std::string &var) {
     char temp[SSTREAM_STRING_SIZE_MAX]{0};
     int32_t size{0};
     read((char *)&size, sizeof(size));
     size = ntoh(size);
     if (size > 0) read(temp, size);
     var = temp;
     return *this;
   }

   stringstream &operator>>(char *var) {
     int32_t size{0};
     read((char *)&size, sizeof(size));
     size = ntoh(size);
     if (size > 0) read(var, size);
     return *this;
   }

 public:
   template <typename T>
   stringstream &operator<<(T var) {
     write((char *)&(var = hton(var)), sizeof(var));
     return *this;
   }

   stringstream &operator<<(int8_t var) {
     write((char *)&var, sizeof(var));
     return *this;
   }

   stringstream &operator<<(uint8_t var) {
     write((char *)&var, sizeof(var));
     return *this;
   }

   stringstream &operator<<(const std::string var) {
     auto size = (int32_t)var.size();
     auto temp = hton(size);
     write((char *)&temp, sizeof(int32_t));
     write(var.c_str(), size);
     return *this;
   }

   stringstream &operator<<(const char *var) {
     auto size = (int32_t)strlen(var);
     auto temp = hton(size);
     write((char *)&temp, sizeof(int32_t));
     write(var, size);
     return *this;
   }

 private:
   char *str_;
   size_t size_;
   size_t cur_;

};

} //namespace plain

#endif //PLAIN_BASIC_STRINGSTREAM_H_
