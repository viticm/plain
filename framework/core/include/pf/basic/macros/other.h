/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id other.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/04 23:33
 * @uses The base define other macros.
*/
#ifndef PF_BASIC_MACROS_OTHER_H_
#define PF_BASIC_MACROS_OTHER_H_

#include "pf/basic/macros/platform.h"

#if OS_WIN //header fix

//disable: "no suitable definition provided for explicit template
//instantiation request" Occurs in VC7 for no justifiable reason on all
//#includes of Singleton
#pragma warning(disable: 4661)

//why use it? for FD_* functions
#pragma warning(disable: 4127)

//for template classes
#pragma warning(disable: 4786)

//disable: "<type> needs to have dll-interface to be used by clients'
//Happens on STL member variables which are not public therefore is ok
#pragma warning (disable: 4251)

//unreachable code
#pragma warning (disable: 4702)

#endif

#if OS_WIN

//check the rename script
#define NOT_RENAME_SOURCE
#ifdef NOT_RENAME_SOURCE
#error "Not rename vc script, are you forgot use the tools/script/bat/rename_forvsbuild.bat?"
#endif //NOT_RENAME_SOURCE

#endif

#if OS_UNIX
#define __stdcall
#endif

enum {
  kAppStatusRunning = 0,
  kAppStatusStop,
};

#define PF_VERSION_MAJOR	"1"
#define PF_VERSION_MINOR	"0"
#define PF_VERSION_NUM		0
#define PF_VERSION_RELEASE	"1"

#define PF_VERSION	"Plain Framework " PF_VERSION_MAJOR "." PF_VERSION_MINOR
#define PF_RELEASE	PF_VERSION "." PF_VERSION_RELEASE
#define PF_COPYRIGHT	PF_RELEASE "  Copyright (C) 2018 by viticm "
#define PF_AUTHORS	"viticm<viticm.ti@gmail.com>"


#endif //PF_BASIC_MACROS_OTHER_H_
