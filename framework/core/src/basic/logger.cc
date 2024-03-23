#include "plain/basic/logger.h"
#include <charconv>
#include <cstdlib>
#include <cassert>
#include <utility>
#include "plain/basic/error.h"
#include "plain/basic/ring.h"
#include "plain/basic/time.h"
#include "plain/basic/global.h"
#include "plain/basic/constants.h"
#include "plain/sys/assert.h"
#include "plain/sys/thread.h"

// PLAIN_SINGLETON_DECL(plain::Logger);

using namespace plain;
using namespace plain::detail::consts;
using Buffer = FixedRing<char, kSmallBufferSize>;

LogLevel init_log_level() {
  if (::getenv("PLAIN_LOG_TRACE"))
    return LogLevel::Trace;
  else if (::getenv("PLAIN_LOG_DEBUG"))
    return LogLevel::Debug;
  else
    return LogLevel::Info;
}

LogLevel Logger::level_ = init_log_level();

void default_output(const std::string_view &log) {
  // FIXME: use complie variable change the globals.
  if (true == GLOBALS["log.print"]) return;
  auto n = fwrite(log.data(), 1, log.size(), stdout);
  assert(n == log.size());
}

void default_flush() {
  if (true == GLOBALS["log.print"]) return;
  fflush(stdout);
}

const char kDigitsHex[] = "0123456789ABCDEF";
static_assert(sizeof kDigitsHex == 17, "wrong number of kDigitsHex");

// FIXME: change it with std::to_chars
std::size_t convert_hex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char *p = buf;

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

const char *LogLevelName[std::to_underlying(LogLevel::Nums)] = {
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

struct Logger::Impl {
  Impl(LogLevel level, const char *filename, uint32_t line,
      int32_t save_errno = 0);
  void finish();
  void format_time();
  time_t time_;
  LogLevel level_;
  // FIXME: use std::source_location replace the filename and line.
  std::string_view filename_;
  uint32_t line_;
  Buffer buffer_;
};

Logger::Impl::Impl(
    LogLevel level, const char *filename, uint32_t line, int32_t save_errno)
  : time_{Time::timestamp()}, level_{level}, filename_{filename}, line_{line} {
  format_time();
  // FIXME: use the complie time char[n] to cache tid.
  auto tid = thread::get_id();
  buffer_.write(tid.c_str(), tid.size());
  buffer_.write(" ", 1);
  buffer_.write(LogLevelName[std::to_underlying(level)], 6);
  if (save_errno != 0) {
    // buffer_.write("errmsg", 10);
    char errno_str[10]{0};
    auto [ptr, ec] = std::to_chars(errno_str, errno_str + 10, save_errno);
    if (std::errc() == ec) {
      buffer_.write(" (errno=", 8);
      buffer_.write(errno_str, ptr - errno_str);
      buffer_.write(") ", 2);
    }
  }
}

void Logger::Impl::format_time() {
  auto time_str = Time::format(true);
  buffer_.write(time_str.data(), time_str.size());
  buffer_.write(" ", 1);
}

void print_log(LogLevel level, const char *str) {
  switch (level) {
    case LogLevel::Fatal:
    case LogLevel::Error:
      io_cerr(str);
      break;
    case LogLevel::Warn:
      io_cwarn(str);
      break;
    case LogLevel::Debug:
      io_cdebug(str);
      break;
    default:
      std::cout << str << std::endl;
  }
}

void Logger::Impl::finish() {
  buffer_.write("-", 1);
  buffer_.write(filename_.data(), filename_.length());
  buffer_.write(":", 1);
  char line[10]{0};
  auto [ptr, ec] = std::to_chars(line, line + sizeof line, line_);
  if (std::errc() == ec) {
    buffer_.write(line, ptr - line);
  }
  if (true == GLOBALS["log.print"]) {
    print_log(level_, buffer_.peek());
  }
  buffer_.write("\n", 1);
}

Logger::Logger(LogLevel level, const std::source_location &location)
  : impl_{std::make_unique<Impl>(
      level, location.file_name(), location.line())} {
  *this << location.function_name() << " ";
}

Logger::Logger(bool abort, const std::source_location &location)
  : impl_{std::make_unique<Impl>(
      abort ? LogLevel::Fatal : LogLevel::Error,
      location.file_name(), location.line(), errno)} {
  *this << location.function_name() << " ";
}

Logger::Logger(const std::string_view &trace,
               const std::source_location &location)
  : impl_{std::make_unique<Impl>(
      LogLevel::Trace, location.file_name(), location.line())} {
  *this << location.function_name() << " ";
  *this << trace.data();
}

Logger::~Logger() {
  impl_->finish();
  auto size = impl_->buffer_.read_avail();
  if (size > 0)
    output_func_({impl_->buffer_.peek(), size});
  if (impl_->level_ == LogLevel::Fatal) {
    flush_func_();
    std::abort();
  }
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

Logger &Logger::operator<<(bool v) {
  impl_->buffer_.write(v ? "1" : "0", 1);
  return *this;
}

Logger &Logger::operator<<(int16_t v) {
  format_integer(v);
  return *this;
}

Logger &Logger::operator<<(uint16_t v) {
  format_integer(v);
  return *this;
}

Logger &Logger::operator<<(int32_t v) {
  format_integer(v);
  return *this;
}

Logger &Logger::operator<<(uint32_t v) {
  format_integer(v);
  return *this;
}

Logger &Logger::operator<<(int64_t v) {
  format_integer(v);
  return *this;
}

Logger &Logger::operator<<(uint64_t v) {
  format_integer(v);
  return *this;
}

Logger &Logger::operator<<(const void* p) {
  if (impl_->buffer_.write_avail() >= kMaxNumericSize) {
    char str[kMaxNumericSize]{0};
    auto v = reinterpret_cast<uintptr_t>(p);
    auto size = convert_hex(str, v);
    impl_->buffer_.write("0x", 2);
    impl_->buffer_.write(str, size);
  }
  return *this;
}

Logger &Logger::operator<<(double v) {
  if (impl_->buffer_.write_avail() >= kMaxNumericSize) {
    char str[kMaxNumericSize]{0};
    auto size = snprintf(str, kMaxNumericSize, "%.12g", v);
    impl_->buffer_.write(str, size);
  }
  return *this;
}

Logger &Logger::operator<<(char v) {
  impl_->buffer_.write(&v, 1);
  return *this;
}

Logger &Logger::operator<<(const char *str) {
  if (str) {
    impl_->buffer_.write(str, strlen(str));
  } else {
    impl_->buffer_.write("(null)", 6);
  }
  return *this;
}

Logger &Logger::operator<<(const std::string &v) {
  impl_->buffer_.write(v.c_str(), v.size());
  return *this;
}

Logger &Logger::operator<<(const Error &e) {
  auto v = e.to_string();
  impl_->buffer_.write(v.c_str(), v.size());
  return *this;
}

void Logger::append(std::string_view &log) {
  impl_->buffer_.write(log.data(), log.size()); 
}

/*
const Buffer& Logger::buffer() const {
  return impl_->buffer_; 
}
*/

void Logger::reset_buffer() {
  impl_->buffer_.producer_clear(); 
}
