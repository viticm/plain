/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id platform.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 18:06
 * @uses The plain framework platform define.
 */
#ifndef PLAIN_BASIC_MACROS_PLATFORM_H_
#define PLAIN_BASIC_MACROS_PLATFORM_H_

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

#endif //PLAIN_BASIC_MACROS_PLATFORM_H_
