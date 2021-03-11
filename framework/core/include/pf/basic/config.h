/**
 * PAP Engine ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/04 21:11
 * @uses The base config.
*/
#ifndef PF_BASIC_CONFIG_H_
#define PF_BASIC_CONFIG_H_

//STD Headers.
/* C { */
#include <cinttypes>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstdarg>
#include <climits>
#include <cstring>
/* } C */

/* C++ { */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <tuple>
#include <regex>
/* } C++ */

/* platform { */
#include "pf/basic/macros/platform.h"
#if OS_WIN
#include <windows.h>
#include <crtdbg.h>
#include <tchar.h>
#include <direct.h>
#include <io.h>
#elif OS_UNIX
#include <sys/types.h>
#include <pthread.h>
#include <execinfo.h>
#include <signal.h>
#include <exception>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#endif
/* } platform */

/* PF useful { */
#include "pf/basic/macros/export.h"
#include "pf/basic/macros/function.h"
#include "pf/basic/macros/limit.h"
#include "pf/basic/macros/other.h"
/* } PF useful */

#endif //PF_BASIC_CONFIG_H_
