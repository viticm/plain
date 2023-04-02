/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 18:02
 * @uses The plain framework basic config header.
 */
#ifndef PLAIN_BASIC_CONFIG_H_
#define PLAIN_BASIC_CONFIG_H_

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
#include <tuple>
#include <regex>
#include <type_traits>
#include <unordered_map>
/* } C++ */

/* platform { */
#include "plain/basic/macros/platform.h"
#include "plain/basic/macros/limit.h"
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

/* PLAIN useful { */
#include "plain/basic/macros/export.h"
#include "plain/basic/macros/function.h"
#include "plain/basic/macros/other.h"
#include "plain/basic/copyable.h"
#include "plain/basic/noncopyable.h"
/* } PLAIN useful */

#endif //PLAIN_BASIC_CONFIG_H_
