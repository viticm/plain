/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id limit.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/08 21:05
 * @uses The base define limit macros.
*/
#ifndef PLAIN_BASIC_MACROS_LIMIT_H_
#define PLAIN_BASIC_MACROS_LIMIT_H_

#include "plain/basic/macros/platform.h"
#include <new>

#ifndef IP_SIZE
#define IP_SIZE 24
#endif

#if OS_WIN
#define HANDLE_INVALID ((VOID*)0)
#elif OS_UNIX
#define HANDLE_INVALID (-1)
#endif

#define ID_INVALID (-1)
#define ID_INVALID_EX (-2)
#define INDEX_INVALID (-1)
#define TAB_PARAM_ID_INVALID (-9999)

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

#if !OS_MAC && defined(__cpp_lib_hardware_interference_size)
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winterference-size"
#endif
constexpr std::size_t kCacheInlineAlignment = 
  std::hardware_destructive_interference_size;
#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#else
constexpr std::size_t kCacheInlineAlignment = 64;
#endif

#endif //PLAIN_BASIC_MACROS_LIMIT_H_
