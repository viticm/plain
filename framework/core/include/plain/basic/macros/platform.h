/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id platform.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 18:06
 * @uses The plain platform define.
 */
#ifndef PLAIN_BASIC_MACROS_PLATFORM_H_
#define PLAIN_BASIC_MACROS_PLATFORM_H_

#include <cstddef> /* For __GLIBC__ */

#if !defined(OS_WIN) || !defined(OS_UNIX) || !defined(OS_MAC) /* { */ //windows上的宏处理与gnu有区别，所以这样定义
#if defined(_MSC_VER) || defined(__ICL) || defined(WIN32) || defined(WIN64) /* { */
#define OS_WIN 1
#elif defined(__GLIBC__) || defined(__GLIBCXX__) || defined(__GNU_LIBRARY__) ||\
      defined(__ANDROID__)
#define OS_UNIX 1
#elif defined(macintosh) || defined(Macintosh) || \
      (defined(__APPLE__) && defined(__MACH__))
#define OS_MAC 1 
#endif /* } */
#endif /* } */


#if !defined(OS_WIN) && !defined(OS_UNIX) && !defined(OS_MAC)
#error "Plain can't found current system type"
#endif

#ifndef OS_UNIX
#define OS_UNIX 0
#endif

#ifndef OS_WIN
#define OS_WIN 0
#endif

#ifndef OS_MAC
#define OS_MAC 0
#endif

#endif //PLAIN_BASIC_MACROS_PLATFORM_H_
