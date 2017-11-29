/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id platform.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/04 21:47
 * @uses The base define platform macros.
 *       OS_WIN: all windows system.
 *       OS_UNIX: all base on unix/linux system.
*/
#ifndef PF_BASIC_MACROS_PLATFORM_H_
#define PF_BASIC_MACROS_PLATFORM_H_

#ifndef OS_WIN /* { */ //windows上的宏处理与gnu有区别，所以这样定义
#if defined(_MSC_VER) || defined(__ICL) || defined(WIN32) || defined(WIN64)
#define OS_WIN 1
#else
#define OS_WIN 0
#endif
#endif /* } */

#ifndef OS_UNIX
#define OS_UNIX !(OS_WIN)
#endif

#endif //PF_BASIC_MACROS_PLATFORM_H_
