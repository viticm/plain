/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id assert.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/01 16:46
 * @uses The assert apis.
 */

#ifndef PLAIN_SYS_ASSERT_H_
#define PLAIN_SYS_ASSERT_H_

#include "plain/sys/config.h"

namespace plain {

PLAIN_API void __assert__(const char *file, 
                       unsigned int line, 
                       const char *func, 
                       const char *expr);
PLAIN_API void __assertex__(const char *file, 
                         unsigned int line, 
                         const char *func, 
                         const char *expr, 
                         const char *msg);
PLAIN_API void __assertspecial__(const char *file, 
                              unsigned int line, 
                              const char *func, 
                              const char *expr, 
                              const char *msg);
PLAIN_API void __messagebox__(const char *msg);
PLAIN_API void __show__(const char *temp);

} //namespace plain

#if defined(NDEBUG)
  #define Assert(expr) ((void)0)
  #define AssertEx(expr,msg) ((void)0)
  #define AssertSpecial(expr,msg) ((void)0)
  #define MyMessageBox(msg) ((void)0)
  #define check_exp(c,e)(e)
#elif OS_UNIX || OS_MAC
  #define Assert(expr) { \
  if (!(expr)) { \
    plain::__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr); \
  }}
  #define AssertEx(expr,msg) { \
  if (!(expr)) { \
    plain::__assertex__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg); \
  }}
  #define AssertSpecial(expr,msg) { \
  if (!(expr)) { \
    plain::__assertspecial__( \
        __FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg); \
  }}
  #define MyMessageBox(msg) ((void)0)
  #define check_exp(c,e)(Assert((c)), (e))
#elif defined(__WIN_CONSOLE__) || defined(__WIN32__) || OS_WIN
  #define Assert(expr) ((void)( \
  (expr) ? 0 : (plain::__assert__(__FILE__,__LINE__,__FUNCTION__,#expr),0)))
  #define AssertEx(expr,msg) ((void)( \
  (expr) ? 0 : (plain::__assertex__( \
      __FILE__,__LINE__,__FUNCTION__,#expr,msg),0)))
  #define AssertSpecial(expr,msg) ((void)( \
  (expr) ? 0 : (plain::__assertspecial__( \
      __FILE__,__LINE__,__FUNCTION__,#expr,msg),0)))
  #define MyMessageBox(msg) __messagebox__(msg)
  #define check_exp(c,e)(Assert(c), (e))
#elif defined(__MFC__)
  #define Assert(expr) ASSERT(expr)
  #define AssertEx(expr,msg) ((void)0)
  #define AssertSpecial(expr,msg) ((void)0)
  #define MyMessageBox(msg) ((void)0)
  #define check_exp(c,e)(e)
#endif

#endif // PLAIN_SYS_ASSERT_H_
