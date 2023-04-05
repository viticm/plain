/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id logger.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 19:54
 * @uses The plain framework logger implemention.
 */
#ifndef PLAIN_BASIC_LOGGER_H_
#define PLAIN_BASIC_LOGGER_H_

#include "plain/basic/config.h"
#include "plain/basic/ring.h"

namespace plain {

// The log level enum define.
enum class LogLevel {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
  Nums,
};

class PLAIN_API Logger /*: public Singleton<Logger>*/ {

using Buffer = FixedRing<char, 4096>;

 public:
   Logger();
   ~Logger();
 
 public:
   // Set log level.
   static void set_level(LogLevel level);
 
   // Get log level.
   static LogLevel get_level();

 public:
  Logger& operator<<(bool v) {
    buffer_.write(v ? "1" : "0", 1);
    return *this;
  }
  
  Logger& operator<<(int16_t);
  Logger& operator<<(uint16_t);
  Logger& operator<<(int32_t);
  Logger& operator<<(uint32_t);
  Logger& operator<<(int64_t);
  Logger& operator<<(uint64_t);
  
  Logger& operator<<(const void*);
  
  Logger& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  Logger& operator<<(double);
  
  Logger& operator<<(char v) {
    buffer_.write(&v, 1);
    return *this;
  }
  
  Logger& operator<<(const char* str) {
    if (str) {
      buffer_.write(str, strlen(str));
    } else {
      buffer_.write("(null)", 6);
    }
    return *this;
  }
  
  Logger& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }
  
  Logger& operator<<(const std::string& v) {
    buffer_.write(v.c_str(), v.size());
    return *this;
  }
 
  void append(const char* data, std::size_t len) { buffer_.write(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void reset_buffer() { buffer_.producer_clear(); }
  
 private:
  void static_check();

  template<typename T>
  void format_integer(T);

 private:
  static LogLevel level_;
  static const int kMaxNumericSize = 48;

 private:
  Buffer buffer_;

};

} // namespace plain

#endif // PLAIN_BASIC_LOGGER_H_
