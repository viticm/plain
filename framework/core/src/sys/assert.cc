#include "plain/sys/assert.h"
#include "plain/basic/io.h"
#include "plain/basic/logger.h"

namespace plain {

void __show__(const char *temp) {
#if OS_UNIX || OS_MAC
  io_cerr("Assert: %s", temp);
#endif
  LOG_WARN << temp;
#if OS_WIN
  static std::mutex mutex;
  std::unique_lock<std::mutex> autolock(mutex);
  ::MessageBoxA(NULL, temp, "异常", MB_OK);
#endif
}

void __messagebox__(const char *msg) {
#if OS_UNIX || OS_MAC
UNUSED(msg);
#endif
#if OS_WIN
  ::MessageBoxA(NULL, msg, "信息", MB_OK);
#endif
}

void __assert__(const char *file, 
                unsigned int line, 
                const char *func , 
                const char *expr) {
  char temp[1024] = {0};
#if OS_UNIX || OS_MAC //换个格式
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
#if OS_UNIX || OS_MAC
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
#if OS_UNIX || OS_MAC
  sprintf(temp, "S[%s][%d][%s][%s]\n[%s]\n", file, line, func, expr, msg) ;
#else
  sprintf(temp, "S[%s][%d][%s][%s]\n[%s]", file, line, func, expr, msg ) ;
#endif
  __show__(temp) ;
}

}
