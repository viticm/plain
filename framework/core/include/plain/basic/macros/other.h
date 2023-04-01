/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id other.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 12:56
 * @uses The plain framework other macros.
 */
#ifndef PLAIN_BASIC_MACROS_OTHER_H_
#define PLAIN_BASIC_MACROS_OTHER_H_

//why use it? for FD_* functions
#pragma warning(disable: 4127)

//for template classes
#pragma warning(disable: 4786)

//disable: "<type> needs to have dll-interface to be used by clients'
//Happens on STL member variables which are not public therefore is ok
#pragma warning (disable: 4251)

//unreachable code
#pragma warning (disable: 4702)

//utf8 no boom(This will disable with cmake)
#pragma warning (disable: 4819)

//API warnings.
#pragma warning (disable: 4996)

#if OS_UNIX
#define __stdcall
#endif

// C++11 and below don't support contexpr as used here.
#if __cplusplus >= 201300
# define PLAIN_CONSTEXPR constexpr 
#else
# define PLAIN_CONSTEXPR
#endif

enum class AppStatus {
  Running = 0,
  Stop,
};

#define PLAIN_VERSION_MAJOR	"1"
#define PLAIN_VERSION_MINOR	"0"
#define PLAIN_VERSION_NUM		0
#define PLAIN_VERSION_RELEASE	"1"

#define PLAIN_VERSION	"Plain Framework " PLAIN_VERSION_MAJOR "." PLAIN_VERSION_MINOR
#define PLAIN_RELEASE	PLAIN_VERSION "." PLAIN_VERSION_RELEASE
#define PLAIN_COPYRIGHT	PLAIN_RELEASE "  Copyright (C) 2023 by viticm "
#define PLAIN_AUTHORS	"viticm<viticm.ti@gmail.com>"

#endif //PLAIN_BASIC_MACROS_OTHER_H_
