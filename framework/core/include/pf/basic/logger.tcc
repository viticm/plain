/**
 * PAP Engine ( https://github.com/viticm/plainframework1 )
 * $Id log.tcc
 * @link https://github.com/viticm/plainframework1 for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com/viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm@126.com/viticm.ti@gmail.com>
 * @date 2016/05/07 22:16
 * @uses Base log moudle template implement.
 */
#ifndef PF_BASIC_LOGGER_TCC_
#define PF_BASIC_LOGGER_TCC_

#include "pf/basic/config.h"
#include "pf/basic/io.tcc"
#include "pf/basic/global.h"
#include "pf/sys/assert.h"
#include "pf/basic/logger.h"

namespace pf_basic {

template <uint8_t type>
void Logger::fast_savelog(const char *logname, const char *format, ...) {
  if (!logids_.isfind(logname) && !register_fastlog(logname)) {
    return;
  }
  uint8_t logid = static_cast<uint8_t>(logids_.get(logname));
  char *cache = logcache_.get(logid);
  if (is_null(cache)) return;
  auto mutex = loglock_.get(logid);
  if (is_null(mutex)) return;
  uint32_t position = log_position_.get(logid);
  char buffer[4096]{0};
  char temp[4096]{0};
  va_list argptr;
  try {
    va_start(argptr, format);
    vsnprintf(temp, sizeof(temp) - 1, format, argptr);
    va_end(argptr);
    if (GLOBALS["log.fast"] == false) { //disable fast log.
      char log_filename[FILENAME_MAX]{0};
      get_log_filename(logname, log_filename);
      slow_savelog<type>(log_filename, temp);
      return;
    }
    char time_str[256]{0};
    get_log_timestr(time_str, sizeof(time_str) - 1);
    snprintf(buffer, sizeof(buffer) - 1,"%s %s", time_str, temp);
  } catch(...) {
    Assert(false);
    return;
  }
  if (GLOBALS["log.print"] == true) {
    switch (type) {
    case 1:
      io_cwarn(buffer);
      break;
    case 2:
      io_cerr(buffer);
      break;
    case 3:
      io_cdebug(buffer);
      break;
    case 9:
      break;
    default:
      printf("%s" LF "", buffer);
      break;
    }
  }
  strncat(buffer, LF, sizeof(LF)); //add wrap
  if (GLOBALS["log.active"] == false) return; //save log condition
  int32_t length = static_cast<int32_t>(strlen(buffer));
  if (length <= 0 || length + position > kDefaultLogCacheSize) return;
  if (GLOBALS["log.singlefile"] == true) {
    //do nothing(one log file is not active now)
  }
  {
    std::unique_lock<std::mutex> autolock(*mutex);
    try {
      memcpy(cache + position, buffer, length);
    } catch(...) {
    //do nogthing
    }
    log_position_.set(logid, position + length);
  }
  if (position + length > (kDefaultLogCacheSize * 2) / 3) {
    flush_log(logname);
  }
}

//模板函数 type 0 普通日志 1 警告日志 2 错误日志 3 调试日志 9 只写日志
template <uint8_t type>
void Logger::slow_savelog(const char *filename_prefix, 
    const char *format, ...) {
  std::unique_lock<std::mutex> autolock(g_log_mutex);
  char buffer[4096]{0};
  char temp[4096]{0};
  va_list argptr;
  try {
    va_start(argptr, format);
    vsnprintf(temp, sizeof(temp) - 1, format, argptr);
    va_end(argptr);
    char time_str[256]{0};
    get_log_timestr(time_str, sizeof(time_str) - 1);
    snprintf(buffer, sizeof(buffer) - 1,"%s %s", time_str, temp);

    if (GLOBALS["log.print"] == true) {
      switch (type) {
        case 1:
        io_cwarn(buffer);
        break;
        case 2:
        io_cerr(buffer);
        break;
        case 3:
        io_cdebug(buffer);
        break;
        case 9:
        break;
        default:
        printf("%s" LF "", buffer);
      }
    }
    strncat(buffer, LF, sizeof(LF)); //add wrap
    if (GLOBALS["log.active"] == 0) return;
    char log_filename[FILENAME_MAX]{0};
    get_log_filename(filename_prefix, log_filename, type);
    FILE* fp;
    fp = fopen(log_filename, "ab");
    if (fp) {
      fwrite(buffer, 1, strlen(buffer), fp);
      fclose(fp);
    }
  } catch(...) {
    io_cerr("pf_basic::Logger::save_log have some log error in here" LF "");
  }
}

}; //namespace pf_basic;

#endif //PF_BASIC_LOGGER_TCC_
