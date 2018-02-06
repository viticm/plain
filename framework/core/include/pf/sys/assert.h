/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id assert.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/18 16:57
 * @uses assert functions
 */
#ifndef PF_SYS_ASSERT_H_
#define PF_SYS_ASSERT_H_

#include "pf/sys/config.h"

namespace pf_sys {

PF_API void __assert__(const char *file, 
                       unsigned int line, 
                       const char *func, 
                       const char *expr);
PF_API void __assertex__(const char *file, 
                         unsigned int line, 
                         const char *func, 
                         const char *expr, 
                         const char *msg);
PF_API void __assertspecial__(const char *file, 
                              unsigned int line, 
                              const char *func, 
                              const char *expr, 
                              const char *msg);
PF_API void __messagebox__(const char *msg);
PF_API void __show__(const char *temp);

} //namespace pf_sys

#if defined(NDEBUG)
  #define Assert(expr) ((void)0)
  #define AssertEx(expr,msg) ((void)0)
  #define AssertSpecial(expr,msg) ((void)0)
  #define MyMessageBox(msg) ((void)0)
  #define check_exp(c,e)(e)
#elif OS_UNIX
  #define Assert(expr) { \
  if (!(expr)) { \
    pf_sys::__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr); \
  }}
  #define AssertEx(expr,msg) { \
  if (!(expr)) { \
    pf_sys::__assertex__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg); \
  }}
  #define AssertSpecial(expr,msg) { \
  if (!(expr)) { \
    pf_sys::__assertspecial__( \
        __FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg); \
  }}
  #define MyMessageBox(msg) ((void)0)
  #define check_exp(c,e)(Assert((c)), (e))
#elif defined(__WIN_CONSOLE__) || defined(__WIN32__) || OS_WIN
  #define Assert(expr) ((void)( \
  (expr) ? 0 : (pf_sys::__assert__(__FILE__,__LINE__,__FUNCTION__,#expr),0)))
  #define AssertEx(expr,msg) ((void)( \
  (expr) ? 0 : (pf_sys::__assertex__( \
      __FILE__,__LINE__,__FUNCTION__,#expr,msg),0)))
  #define AssertSpecial(expr,msg) ((void)( \
  (expr) ? 0 : (pf_sys::__assertspecial__( \
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

#endif //PF_SYS_ASSERT_H_
