/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id api.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2023/04/01 21:05
 * @uses file extend apis
 */
#ifndef PLAIN_FILE_API_H_
#define PLAIN_FILE_API_H_

#include "plain/file/config.h"

#if OS_UNIX /* { */
#include <sys/types.h>  // for open()
#include <sys/stat.h>   // for open()
#include <unistd.h>     // for fcntl()
#include <fcntl.h>      // for fcntl()
#include <sys/ioctl.h>  // for ioctl()
#include <errno.h>      // for errno
#elif OS_WIN /* }{ */
#include <io.h>         // for _open()
#include <fcntl.h>      // for _open()/_close()/_read()/_write()...
#endif /* } */

namespace plain {

PLAIN_API int32_t open(const char *filename, int32_t flag);
PLAIN_API int32_t openmode(const char *filename, int32_t flag, int32_t mode);
PLAIN_API void close(int32_t fd);
PLAIN_API uint32_t read(int32_t fd, void* buffer, uint32_t length);
PLAIN_API uint32_t write(int32_t fd, const void* buffer, uint32_t length);
PLAIN_API int32_t fcntl(int32_t fd, int32_t cmd);
PLAIN_API int32_t fcntlarg(int32_t fd, int32_t cmd, int32_t arg);
PLAIN_API bool get_nonblocking(int32_t socketid);
PLAIN_API bool set_nonblocking(int32_t socketid, bool on);
PLAIN_API int32_t ioctl(int32_t fd, int32_t request, void *argp);
PLAIN_API uint32_t available(int32_t fd);
PLAIN_API int32_t dup(int32_t fd);
PLAIN_API int64_t lseek(int32_t fd, uint64_t offset, int32_t whence);
PLAIN_API int64_t tell(int32_t fd);
PLAIN_API bool truncate(const char *filename);

} //namespace plain

#endif //PLAIN_FILE_API_H_
