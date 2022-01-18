/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id export.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/04 21:51
 * @uses The base define export macros.
 *       Refer: lua codes.
*/
#ifndef PF_BASIC_MACROS_EXPORT_H_
#define PF_BASIC_MACROS_EXPORT_H_

#include "pf/basic/macros/platform.h"

// Compatible for anthor defined for dll.
#if OS_WIN && (defined(PF_LINKED_AS_SHARED_LIBRARY) || \
  defined(PF_CREATE_SHARED_LIBRARY)) /* { */

// Defined macro.
#ifndef PF_BUILD_AS_DLL
#define PF_BUILD_AS_DLL 1
#endif
#if defined(PF_CREATE_SHARED_LIBRARY) && !defined(PF_CORE)
#define PF_CORE 1
#endif

#endif /* } */

/*
@@ PF_API is a mark for all core API functions.
@@ PFLIB_API is a mark for all auxiliary library functions.
@@ PFMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** PF_BUILD_AS_DLL to get it).
*/

#if OS_WIN && defined(PF_BUILD_AS_DLL) && !defined(PF_USE_LIB) /* { */ 

#if defined(PF_CORE) || defined(PF_LIB) /* { */
#define PF_API __declspec(dllexport)
#else /* }{ */
#define PF_API __declspec(dllimport)
#endif

#else  /* }{ */ 

#define PF_API

#endif /* } */

#if OS_WIN && \
  defined(PF_PLUGIN_BUILD_AS_DLL) && \
  !defined(PF_PLUGIN_USE_LIB) /* { */ 

#if defined(PF_PLUGIN_CORE) || defined(PF_PLUGIN_LIB) /* { */
#define PF_PLUGIN_API __declspec(dllexport)
#else /* }{ */
#define PF_PLUGIN_API __declspec(dllimport)
#endif

#else  /* }{ */ 

#define PF_PLUGIN_API

#endif /* } */

/* more often than not the libs go together with the core */
#define PFLIB_API  PF_API
#define PFMOD_API  PFLIB_API

/*
@@ PFI_FUNC is a mark for all extern functions that are not to be
@* exported to outside modules.
@@ PFI_DDEF and PFI_DDEC are marks for all extern (const) variables
@* that are not to be exported to outside modules (PFI_DDEF for
@* definitions and PFI_DDEC for declarations).
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
#define PFI_FUNC	__attribute__((visibility("hidden"))) extern
#define PFI_DDEC	PFI_FUNC
#define PFI_DDEF	/* empty */

#else				/* }{ */
#define PFI_FUNC
#define PFI_DDEC
#define PFI_DDEF	/* empty */
#endif				/* } */

//#define PF_OPEN_EPOLL //test

#endif //PF_BASIC_MACROS_EXPORT_H_
