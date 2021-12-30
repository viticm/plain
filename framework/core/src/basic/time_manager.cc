#include "pf/sys/assert.h"
#include "pf/basic/io.tcc"
#include "pf/basic/time_manager.h"

std::unique_ptr< pf_basic::TimeManager > g_time_manager{nullptr};

int32_t g_file_name_fix = 0;
uint64_t g_file_name_fix_last = 0;

namespace pf_basic {

template<> TimeManager *Singleton<TimeManager>::singleton_ = nullptr;
TimeManager *TimeManager::getsingleton_pointer() {
  return singleton_;
}

TimeManager &TimeManager::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

TimeManager::TimeManager() :
  start_time_{0},
  current_time_{0} {
}

TimeManager::~TimeManager() {
  //do nothing
}

bool TimeManager::init() {
  s_time_ = std::chrono::steady_clock::now();
#if OS_WIN
  start_time_ = GetTickCount();
  current_time_ = GetTickCount();
#elif OS_UNIX
  start_time_ = 0;
  current_time_ = 0;
  // gettimeofday(&start_, &time_zone_);
#endif
  reset_time();
  g_file_name_fix = get_day_time();
  g_file_name_fix_last = get_tickcount();
  return true;
}

// 多线程下获取这个方法会造成后面获取的时间会小于之前的时间
// 猜测是end_在多线程下其数值未加锁造成？
/*
uint32_t TimeManager::get_tickcount() {
#if OS_WIN
  current_time_ = GetTickCount();
#elif OS_UNIX
  gettimeofday(&end_, &time_zone_);
  double time1 = static_cast<double>(start_.tv_sec * 1000) +
           static_cast<double>(start_.tv_usec / 1000);
  double time2 = static_cast<double>(end_.tv_sec * 1000) +
           static_cast<double>(end_.tv_usec / 1000);
  current_time_ = static_cast<uint32_t>(time2 - time1);
#endif
  return current_time_;
}
*/

uint64_t TimeManager::get_tickcount() {
  using namespace std::chrono;
  auto now = steady_clock::now();
  auto passed = now - s_time_;
  auto r = duration_cast<milliseconds>(passed).count();
  current_time_ = static_cast<uint64_t>(r);
  return current_time_;
}

uint32_t TimeManager::get_current_time() {
  reset_time();
  uint32_t time;
  tm_totime(&tm_, time);
  return time;
}

uint64_t TimeManager::get_start_time() const {
  return start_time_;
}

uint64_t TimeManager::get_saved_time() const {
  return current_time_;
}

void TimeManager::reset_time() {
  time_t newtime;
  if (time(&newtime) != static_cast<time_t>(-1)) {
    set_time_ = newtime;
  }
#if OS_WIN
  tm *newtm = localtime(&set_time_);
  tm_ = *newtm;
#elif OS_UNIX
  tm newtm;
  tm *_tm = localtime_r(&set_time_, &newtm);
  if (_tm) tm_ = newtm;
#endif
}

time_t TimeManager::get_ansi_time() {
  reset_time();
  return set_time_;
}

uint32_t TimeManager::get_ctime() {
  time_t currenttime = get_ansi_time();
  uint32_t result = static_cast<uint32_t>(currenttime);
  return result;
}

void TimeManager::get_full_format_time(char *format_time, uint32_t length) {
  reset_time();
  strftime(format_time, length, "%Y-%m-%d %H:%M:%S", &tm_);
}

uint16_t TimeManager::get_year() {
  return static_cast<uint16_t>(tm_.tm_year + 1900);
}

uint8_t TimeManager::get_month() {
  return static_cast<uint8_t>(tm_.tm_mon + 1);
}

uint8_t TimeManager::get_day() {
  return static_cast<uint8_t>(tm_.tm_mday);
}

uint8_t TimeManager::get_hour() {
  return static_cast<uint8_t>(tm_.tm_hour);
}

uint8_t TimeManager::get_minute() {
  return static_cast<uint8_t>(tm_.tm_min);
}

uint8_t TimeManager::get_second() {
  return static_cast<uint8_t>(tm_.tm_sec);
}

uint8_t TimeManager::get_week() {
  return static_cast<uint8_t>(tm_.tm_wday);
}

uint32_t TimeManager::tm_todword() {
  reset_time();
  uint32_t result = 0;
  result += get_year();
  result -= 2000;
  result *= 100;
  result += get_month();
  result *= 100;
  result += get_day();
  result = result * 100;
  result += get_hour();
  result *= 100;
  result += get_minute();
  return result;
}

void TimeManager::dword_totm(uint32_t time, tm* _tm) {
  Assert(_tm);
  memset(_tm, 0 , sizeof(*_tm));
  _tm->tm_year = (time / 100000000) + 2000 - 1900;
  _tm->tm_mon = (time % 100000000) / 1000000 - 1;
  _tm->tm_mday = (time % 1000000) / 10000;
  _tm->tm_hour = (time % 10000) / 100 ;
  _tm->tm_min  = time % 100;
}

uint32_t TimeManager::diff_dword_time(uint32_t time1, uint32_t time2) {
  tm _tm1, _tm2;
  dword_totm(time1, &_tm1);
  dword_totm(time2, &_tm2);
  time_t _time1, _time2;
  _time1 = mktime(&_tm1);
  _time2 = mktime(&_tm2);
  uint32_t result = 
    static_cast<uint32_t>(
      (abs(static_cast<int32_t>(difftime(_time2,_time1) / 60))));
  return result;
}

int32_t TimeManager::diff_day_count(time_t ansi_time1, time_t ansi_time2) {
  int32_t result = 
    static_cast<int32_t>(difftime(ansi_time1,ansi_time2) / (24 * 60 * 60));
  return result;
}

uint32_t TimeManager::get_day_time() {
  uint32_t result = 0;
  result += get_year();
  result *= 100;
  result += get_month();
  result *= 100;
  result += get_day();
  return result;
}

uint64_t TimeManager::get_run_time() {
  get_tickcount();
  auto result = current_time_ - start_time_;
  return result;
}

void TimeManager::time_totm(uint32_t time, tm* _tm) {
  Assert(_tm);
  memset(_tm, 0, sizeof(*_tm));
  _tm->tm_year = (time >> 24) & 0xff;
  _tm->tm_mon  = (time >> 20) & 0xf;
  _tm->tm_mday = (time >> 15) & 0x1f;
  _tm->tm_hour = (time >> 10) & 0x1f;
  _tm->tm_min  = (time >> 4) & 0x3f;
  _tm->tm_sec  = (time) & 0xf;
}

void TimeManager::tm_totime(tm* _tm, uint32_t &time) {
  Assert(_tm);
  time = 0;
  time += ((_tm->tm_year) & 0xff) << 24;
  time += ((_tm->tm_mon) & 0xf) << 20;
  time += ((_tm->tm_mday) & 0x1f) << 15;
  time += ((_tm->tm_hour) & 0x1f) << 10;
  time += ((_tm->tm_min) & 0x3f) << 4;
  time += ((_tm->tm_sec) & 0xf);
}

uint32_t TimeManager::diff_time(uint32_t time1, uint32_t time2) {
  tm _tm1, _tm2;
  time_totm(time1, &_tm1);
  time_totm(time2, &_tm2);
  time_t timefirst = 0, timenext = 0;
  timefirst = mktime(&_tm1);
  timenext = mktime(&_tm2);
  uint32_t result = static_cast<uint32_t>(
    abs(static_cast<int32_t>(
    difftime(timefirst, timenext))) * 1000);
  return result;
}

uint32_t TimeManager::get_days() {
  uint32_t result = 0;
  time_t _time_t;
  time(&_time_t);
  tm* _tm = localtime(&set_time_);
  result = (_tm->tm_year - 100) * 1000;
  result += _tm->tm_yday;
  return result;
}

uint32_t TimeManager::get_hours() {
  uint32_t result = 0;
  time_t _time_t;
  time(&_time_t);
  tm* _tm = localtime(&set_time_);
  if (2008 == _tm->tm_year + 1900) {
    result = 365;
  }
  result += _tm->tm_yday;
  result *= 100;
  result += _tm->tm_hour * 4;
  result += static_cast<uint32_t>(_tm->tm_min / 15);
  return result;
}

uint32_t TimeManager::get_weeks() {
  uint32_t result = 0;
  time_t _time_t;
  time(&_time_t);
  tm* _tm = localtime(&set_time_);
  result  = (_tm->tm_year - 100) * 1000;
  if (_tm->tm_yday <= _tm->tm_wday) return result;
  int32_t diff = _tm->tm_yday - _tm->tm_wday;
  result += static_cast<uint32_t>(ceil(static_cast<double>(diff / 7)));
  return result;
}

} //namespace pf_basic
