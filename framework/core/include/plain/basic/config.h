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
#include <cstring>
/* } C */

/* C++ { */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <future>
#include <functional>
#include <stdexcept>
#include <memory>
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
#endif
/* } platform */

/* PLAIN useful { */
#include "plain/basic/macros/export.h"
#include "plain/basic/macros/function.h"
#include "plain/basic/macros/other.h"
#include "plain/basic/copyable.h"
#include "plain/basic/noncopyable.h"
/* } PLAIN useful */

namespace plain {
class Error;
};

#endif //PLAIN_BASIC_CONFIG_H_
