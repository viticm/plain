#ifdef OS_UNIX
#include <execinfo.h>
#endif
#include <time.h>
#include "pf/basic/time_manager.h"
#include "pf/basic/logger.h"
#include "pf/basic/util.h"
#include "pf/basic/io.tcc"
#include "pf/sys/thread.h"
#include "pf/sys/assert.h"

namespace pf_sys {

void __show__(const char *temp) {
#if OS_UNIX
  pf_basic::io_cerr("Assert: %s", temp);
#endif
  SLOW_WRITELOG("assert", "%s", temp);
#if OS_WIN
  static std::mutex mutex;
  std::unique_lock<std::mutex> autolock(mutex);
  ::MessageBoxA(NULL, temp, "异常", MB_OK);
#endif
}

void __messagebox__(const char *msg) {
#if OS_WIN
  ::MessageBoxA(NULL, msg, "信息", MB_OK);
#endif
}

void __assert__(const char *file, 
                unsigned int line, 
                const char *func , 
                const char *expr) {
  char temp[1024] = {0};
#if OS_UNIX //换个格式
  sprintf(temp, "[%s][%d][%s][%s]\n", file, line, func, expr);
#else
  sprintf(temp, "[%s][%d][%s][%s]", file, line, func, expr);
#endif
  __show__(temp);
}

void __assertex__(const char *file, 
                  unsigned int line, 
                  const char *func, 
                  const char *expr,
                  const char *msg) {
  char temp[1024] = {0};
#if OS_UNIX
  sprintf(temp, "[%s][%d][%s][%s]\n[%s]\n", file, line, func, expr, msg);
#else
  sprintf(temp, "[%s][%d][%s][%s]\n[%s]", file, line, func, expr, msg);
#endif
  __show__(temp);
}

void __assertspecial__(const char *file, 
                       unsigned int line, 
                       const char *func, 
                       const char *expr,
                       const char *msg) {
  char temp[1024] = {0};
#if OS_UNIX
  sprintf(temp, "S[%s][%d][%s][%s]\n[%s]\n", file, line, func, expr, msg) ;
#else
  sprintf(temp, "S[%s][%d][%s][%s]\n[%s]", file, line, func, expr, msg ) ;
#endif
  __show__(temp) ;
}

} //namespace pf_sys
