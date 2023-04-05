#include <chrono>
#include <iomanip>
#include "plain/basic/time.h"

PLAIN_SINGLETON_DECL(plain::Time);

using namespace plain;

Time::Time() :
  s_time_{std::chrono::steady_clock::now()} {
  // do nothing
}

Time::~Time() = default;

uint32_t Time::timestamp() {
  auto now = std::chrono::system_clock::now();
  auto now_s = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto value = now_s.time_since_epoch();
  return value.count();
}

uint64_t Time::tickcount() const noexcept {
  using namespace std::chrono;
  auto now = steady_clock::now();
  auto passed = now - s_time_;
  return static_cast<uint64_t>(duration_cast<milliseconds>(passed).count());
}

std::string Time::format() {
  auto now = std::chrono::system_clock::now();
  auto tm = std::chrono::system_clock::to_time_t(now);
  std::ostringstream os;
  os << std::put_time(std::localtime(&tm), "%F %T");
  return os.str();
}

std::string Time::format(bool show_microseconds) {
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  auto value = now_ms.time_since_epoch();
  auto tm = std::chrono::system_clock::to_time_t(now);
  std::ostringstream os;
  os << std::put_time(std::localtime(&tm), "%F %T");
  if (show_microseconds) {
    os << "." << std::setfill('0') << std::setw(3);
    os << (value % 1000);
  }
  return os.str();
}

uint64_t Time::nanoseconds() {
  auto now = std::chrono::steady_clock::now();
  auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto value = now_ns.time_since_epoch();
  return value.count();
}
