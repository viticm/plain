/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id export.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 18:04
 * @uses your description
 */
#ifndef PLAIN_BASIC_MACROS_EXPORT_H_
#define PLAIN_BASIC_MACROS_EXPORT_H_

#include "plain/basic/macros/platform.h"

// Compatible for anthor defined for dll.
#if OS_WIN && (defined(PLAIN_LINKED_AS_SHARED_LIBRARY) || \
  defined(PLAIN_CREATE_SHARED_LIBRARY)) /* { */

// Defined macro.
#ifndef PLAIN_BUILD_AS_DLL
#define PLAIN_BUILD_AS_DLL 1
#endif
#if defined(PLAIN_CREATE_SHARED_LIBRARY) && !defined(PLAIN_CORE)
#define PLAIN_CORE 1
#endif

#endif /* } */

/*
@@ PLAIN_API is a mark for all core API functions.
@@ PLAINLIB_API is a mark for all auxiliary library functions.
@@ PLAINMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** PLAIN_BUILD_AS_DLL to get it).
*/

#if OS_WIN && defined(PLAIN_BUILD_AS_DLL) && !defined(PLAIN_USE_LIB) /* { */ 

#if defined(PLAIN_CORE) || defined(PLAIN_LIB) /* { */
#define PLAIN_API __declspec(dllexport)
#else /* }{ */
#define PLAIN_API __declspec(dllimport)
#endif

#else  /* }{ */ 

#define PLAIN_API

#endif /* } */

#if OS_WIN && \
  defined(PLAIN_PLUGIN_BUILD_AS_DLL) && \
  !defined(PLAIN_PLUGIN_USE_LIB) /* { */ 

#if defined(PLAIN_PLUGIN_CORE) || defined(PLAIN_PLUGIN_LIB) /* { */
#define PLAIN_PLUGIN_API __declspec(dllexport)
#else /* }{ */
#define PLAIN_PLUGIN_API __declspec(dllimport)
#endif

#else  /* }{ */ 

#define PLAIN_PLUGIN_API

#endif /* } */

/* more often than not the libs go together with the core */
#define PLAINLIB_API  PLAIN_API
#define PLAINMOD_API  PLAINLIB_API

/*
@@ PLAINI_FUNC is a mark for all extern functions that are not to be
@* exported to outside modules.
@@ PLAINI_DDEF and PLAINI_DDEC are marks for all extern (const) variables
@* that are not to be exported to outside modules (PLAINI_DDEF for
@* definitions and PLAINI_DDEC for declarations).
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when Lua is compiled as a shared library. Not all elf targets support
** this attribute. Unfortunately, gcc does not offer a way to check
** whether the target offers that support, and those without support
** give a warning about it. To avoid these warnings, change to the
** default definition.
*/
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
  defined(__ELF__)		/* { */
#define PLAINI_FUNC	__attribute__((visibility("hidden"))) extern
#define PLAINI_DDEC	PLAINI_FUNC
#define PLAINI_DDEF	/* empty */

#else				/* }{ */
#define PLAINI_FUNC
#define PLAINI_DDEC
#define PLAINI_DDEF	/* empty */
#endif				/* } */

//#define PLAIN_OPEN_EPOLL //test

#endif //PLAIN_BASIC_MACROS_EXPORT_H_
