/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id util.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 19:11
 * @uses The plain sys util functions.
 */
#ifndef PLAIN_SYS_UTIL_H_
#define PLAIN_SYS_UTIL_H_

#include "plain/sys/config.h"

namespace plain::util {

PLAIN_API int32_t exec(const char *command, char *result, size_t size);
PLAIN_API bool set_core_rlimit();

} // namespace plain::util

#endif // PLAIN_SYS_UTIL_H_
