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
#include <source_location>
#include <string_view>

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

using output_func_t = std::function<void(const std::string_view &)>;
using flush_func_t = std::function<void()>;

 public:
  
  explicit Logger(LogLevel level = LogLevel::Info,
      const std::source_location& location = std::source_location::current());
  explicit Logger(bool abort,
      const std::source_location& location = std::source_location::current());
  ~Logger();
 
 public:
  // Set log level.
  static void set_level(LogLevel level) {
    level_ = level;
  }
 
  // Get log level.
  static LogLevel get_level() {
    return level_;
  }

  // Set output function.
  static void set_output(output_func_t func) {
    output_func_ = func;
  }

  // Set flush function.
  static void set_flush(flush_func_t func) {
    flush_func_ = func;
  }

 public:
  Logger& operator<<(bool v);
  
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
  
  Logger& operator<<(char v);
  
  Logger& operator<<(const char *str); 
  Logger& operator<<(const unsigned char *str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }
  
  Logger& operator<<(const std::string &v);

  void append(std::string_view &log);
  void reset_buffer();
  
 private:
  void static_check();

  template<typename T>
  void format_integer(T);

 private:
  static LogLevel level_;
  static const int kMaxNumericSize = 48;
  static output_func_t output_func_;
  static flush_func_t flush_func_;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace plain

#define LOG_TRACE if (plain::Logger::get_level() <= plain::LogLevel::Trace) \
  plain::Logger(plain::LogLevel::Trace)
#define LOG_DEBUG if (plain::Logger::get_level() <= plain::LogLevel::Trace) \
  plain::Logger(plain::LogLevel::Debug)
#define LOG_INFO if (plain::Logger::get_level() <= plain::LogLevel::Info) \
  plain::Logger()
#define LOG_WARN plain::Logger(plain::LogLevel::Warn)
#define LOG_ERROR plain::Logger(plain::LogLevel::Error)
#define LOG_SYSERR plain::Logger(false)
#define LOG_SYSFATAL plain::Logger(true)

#endif // PLAIN_BASIC_LOGGER_H_
