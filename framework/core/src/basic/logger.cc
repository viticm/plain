#include "plain/basic/logger.h"
#include <charconv>
#include "plain/basic/ring.h"
#include "plain/basic/time.h"
#include "plain/basic/traits.h"
#include "plain/sys/assert.h"
#include "plain/sys/thread.h"

// PLAIN_SINGLETON_DECL(plain::Logger);

using namespace plain;
using Buffer = FixedRing<char, 4096>;

LogLevel init_log_level() {
  if (::getenv("PLAIN_LOG_TRACE"))
    return LogLevel::Trace;
  else if (::getenv("PLAIN_LOG_DEBUG"))
    return LogLevel::Debug;
  else
    return LogLevel::Info;
}

LogLevel Logger::level_ = init_log_level();

void default_output(const char* msg, std::size_t len) {
  auto n = fwrite(msg, 1, len, stdout);
  Assert(n == len);
}

void default_flush() {
  fflush(stdout);
}

const char kDigitsHex[] = "0123456789ABCDEF";
static_assert(sizeof kDigitsHex == 17, "wrong number of kDigitsHex");

std::size_t convert_hex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char* p = buf;

  do {
    int32_t lsd = static_cast<int32_t>(i % 16);
    i /= 16;
    *p++ = kDigitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

Logger::output_func_t Logger::output_func_{default_output};
Logger::flush_func_t Logger::flush_func_{default_flush};

const char* LogLevelName[to_underlying_t(LogLevel::Nums)] = {
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

struct Logger::Impl {
  Impl(LogLevel level, const char* filename, uint16_t errno, uint32_t line);
  void flush();
  void format_time();
  uint32_t time_;
  LogLevel level_;
  const char* filename_;
  uint32_t line_;
  Buffer buffer_;
};

Logger::Impl::Impl(
    LogLevel level, const char *filename, uint16_t errno, uint32_t line)
  : time_{Time::timestamp()}, level_{level}, filename_{filename}, line_{line} {
  format_time();
  // FIXME: use the complie time char[n] to cache tid.
  auto tid = thread::get_id();
  buffer_.write(tid.c_str(), tid.size());
  buffer_.write(" ", 1);
  buffer_.write(LogLevelName[to_underlying_t(level)], 6);
  if (errno != 0) {
    // buffer_.write("errmsg", 10);
    char errno_str[10]{0};
    auto [ptr, ec] = std::to_chars(errno_str, errno_str + 10, errno);
    buffer_.write(" (errno=", 8);
    buffer_.write(errno_str, ptr - errno_str);
    buffer_.write(") ", 2);
  }
}

void Logger::Impl::format_time() {
  auto time_str = Time::format(true);
  buffer_.write(time_str.data(), time_str.size());
  buffer_.write(" ", 1);
}

void Logger::static_check() {
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,
                "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,
                "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<int64_t>::digits10,
                "kMaxNumericSize is large enough");
}

template <typename T>
void Logger::format_integer(T value) {
  if (impl_->buffer_.write_avail() >= kMaxNumericSize) {
    char str[kMaxNumericSize]{0};
    auto [ptr, ec] = std::to_chars(str, str + sizeof str, value);
    impl_->buffer_.write(str, ptr - str);
  }
}

Logger& Logger::operator<<(bool v) {
  impl_->buffer_.write(v ? "1" : "0", 1);
  return *this;
}

Logger& Logger::operator<<(int16_t v) {
  format_integer(v);
  return *this;
}

Logger& Logger::operator<<(uint16_t v) {
  format_integer(v);
  return *this;
}

Logger& Logger::operator<<(int32_t v) {
  format_integer(v);
  return *this;
}

Logger& Logger::operator<<(uint32_t v) {
  format_integer(v);
  return *this;
}

Logger& Logger::operator<<(int64_t v) {
  format_integer(v);
  return *this;
}

Logger& Logger::operator<<(uint64_t v) {
  format_integer(v);
  return *this;
}

Logger& Logger::operator<<(const void* p) {
  if (impl_->buffer_.write_avail() >= kMaxNumericSize) {
    char str[kMaxNumericSize]{0};
    auto v = reinterpret_cast<uintptr_t>(p);
    auto size = convert_hex(str, v);
    impl_->buffer_.write("0x", 2);
    impl_->buffer_.write(str, size);
  }
  return *this;
}

Logger& Logger::operator<<(double v) {
  if (impl_->buffer_.write_avail() >= kMaxNumericSize) {
    char str[kMaxNumericSize]{0};
    auto size = snprintf(str, kMaxNumericSize, "%.12g", v);
    impl_->buffer_.write(str, size);
  }
  return *this;
}

Logger& Logger::operator<<(char v) {
  impl_->buffer_.write(&v, 1);
  return *this;
}

Logger& Logger::operator<<(const char* str) {
  if (str) {
    impl_->buffer_.write(str, strlen(str));
  } else {
    impl_->buffer_.write("(null)", 6);
  }
  return *this;
}

Logger& Logger::operator<<(const std::string& v) {
  impl_->buffer_.write(v.c_str(), v.size());
  return *this;
}

void Logger::append(const char* data, std::size_t len) {
  impl_->buffer_.write(data, len); 
}

/*
const Buffer& Logger::buffer() const {
  return impl_->buffer_; 
}
*/

void Logger::reset_buffer() {
  impl_->buffer_.producer_clear(); 
}
