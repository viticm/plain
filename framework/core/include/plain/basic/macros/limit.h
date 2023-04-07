/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id limit.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/04 22:27
 * @uses The base define limit macros.
*/
#ifndef PLAIN_BASIC_MACROS_LIMIT_H_
#define PLAIN_BASIC_MACROS_LIMIT_H_

#include "pf/basic/macros/platform.h"

#ifndef IP_SIZE
#define IP_SIZE 24
#endif

#ifndef FD_SETSIZE 
#define FD_SETSIZE 4906
#endif

#define LOGTYPE_MAX 100

#if OS_WIN
#define HANDLE_INVALID ((VOID*)0)
#elif OS_UNIX
#define HANDLE_INVALID (-1)
#endif

#define ID_INVALID (-1)
#define ID_INVALID_EX (-2)
#define INDEX_INVALID (-1)
#define TAB_PARAM_ID_INVALID (-9999)

#ifndef UCHAR_MAX
#define UCHAR_MIN (0)
#define UCHAR_MAX (0xFF)
#endif

#ifndef BYTE_MAX
#define BYTE_MIN UCHAR_MIN
#define BYTE_MAX UCHAR_MAX
#endif

#ifndef LF
#if OS_WIN
#define LF "\n"
#elif OS_UNIX
#define LF "\r\n"
#endif
#endif

#ifndef SSTREAM_STRING_SIZE_MAX
#define SSTREAM_STRING_SIZE_MAX (10240)
#endif

#endif //PLAIN_BASIC_MACROS_LIMIT_H_
