#include "plain/file/api.h"

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

int32_t open(const char *filename, int32_t flag) {
#if OS_UNIX
  int32_t fd = ::open(filename, flag);
#elif OS_WIN
  int32_t fd = ::_open(filename, flag);
#endif
  if (fd < 0) {
#if OS_UNIX
    switch (errno) {
      case EEXIST : 
      case ENOENT : 
      case EISDIR : 
      case EACCES : 
      case ENAMETOOLONG : 
      case ENOTDIR : 
      case ENXIO : 
      case ENODEV : 
      case EROFS : 
      case ETXTBSY : 
      case EFAULT : 
      case ELOOP : 
      case ENOSPC : 
      case ENOMEM : 
      case EMFILE : 
      case ENFILE : 
      default : {
        break;
      }
    }//end of switch
#elif OS_WIN
  // ...
#endif
  }
  return fd;
}

int32_t openmode(const char * filename, int32_t flag, int32_t mode) {
  int32_t fd{-1};
#if OS_UNIX
  fd = ::open(filename, flag, mode);
#elif OS_WIN
  fd = ::_open(filename, flag, mode);
#endif

  if (fd < 0) {
#if OS_UNIX
    switch (errno) {
      case EEXIST : 
      case EISDIR : 
      case EACCES : 
      case ENAMETOOLONG : 
      case ENOENT : 
      case ENOTDIR : 
      case ENXIO : 
      case ENODEV : 
      case EROFS : 
      case ETXTBSY : 
      case EFAULT : 
      case ELOOP : 
      case ENOSPC : 
      case ENOMEM : 
      case EMFILE : 
      case ENFILE : 
      default : {
        break;
      }
    }//end of switch
#elif OS_WIN
  // ...
#endif
  }
  return fd;
}

uint32_t read(int32_t fd, void *buffer, uint32_t length) {
#if OS_UNIX
  int32_t result = ::read(fd, buffer, length);
#elif OS_WIN
  int32_t result = ::_read (fd, buffer, length);
#endif
  if (result < 0) {

#if OS_UNIX
    switch (errno) {
      case EINTR : 
      case EAGAIN : 
      case EBADF : 
      case EIO : 
      case EISDIR : 
      case EINVAL : 
      case EFAULT : 
      case ECONNRESET :
      default : {
        break;
      }
    }
#elif OS_WIN
  // ...
#endif
  } 
  else if (0 == result) {

  }
  return result;
}

uint32_t write(int32_t fd, const void *buffer, uint32_t length) {
#if OS_UNIX
  int32_t result = ::write(fd, buffer, length);
#elif OS_WIN
  int32_t result = ::_write(fd, buffer, length);
#endif

  if (result < 0) {
    
#if OS_UNIX
    switch (errno) {
      case EAGAIN : 
      case EINTR : 
      case EBADF : 
      case EPIPE : 
      case EINVAL: 
      case EFAULT: 
      case ENOSPC : 
      case EIO : 
      case ECONNRESET :
      default : {
          break;
      }
    }
#elif OS_WIN
  //...
#endif
  }
  return result;
}


void close(int32_t fd) {
 
#if OS_UNIX
  ::close(fd);
  switch ( errno ) {
    case EBADF : 
    default : {
        break;
    }
  }
#elif OS_WIN
  ::_close(fd);
#endif
}

int32_t fcntl([[maybe_unused]] int32_t fd, [[maybe_unused]] int32_t cmd) {
#if OS_UNIX
  int32_t result = ::fcntl(fd, cmd);
  if (result < 0) {
    switch (errno) {
      case EINTR : 
      case EBADF : 
      case EACCES : 
      case EAGAIN : 
      case EDEADLK : 
      case EMFILE : 
      case ENOLCK : 
      default : {
        break;
      }
    }
  }
  return result;
#elif OS_WIN
  return 0 ;
#endif
}

int32_t fcntlarg(
    [[maybe_unused]] int32_t fd,
    [[maybe_unused]] int32_t cmd,
    [[maybe_unused]] int32_t arg) {
#if OS_UNIX
  int32_t result = ::fcntl(fd, cmd, arg);
  if (result < 0) {
    switch (errno) {
      case EINTR : 
      case EINVAL : 
      case EBADF : 
      case EACCES : 
      case EAGAIN : 
      case EDEADLK : 
      case EMFILE : 
      case ENOLCK : 
      default : {
        break;
      }
    }
  }
  return result;
#elif OS_WIN
  return 0 ;
#endif
}

bool get_nonblocking([[maybe_unused]] int32_t fd) {
#if OS_UNIX
  int32_t flag = fcntlarg(fd, F_GETFL, 0);
  return flag & O_NONBLOCK;
#elif OS_WIN
  return false;
#endif
}

bool set_nonblocking([[maybe_unused]] int32_t fd, [[maybe_unused]] bool on) {
  bool r{false};
#if OS_UNIX
  int32_t flag = fcntlarg(fd, F_GETFL, 0);
  if (on)
    // make nonblocking fd
    flag |= O_NONBLOCK;
  else
    // make blocking fd
    flag &= ~O_NONBLOCK;
  auto e = fcntlarg(fd, F_SETFL, flag);
  r = e >= 0;
#elif OS_WIN
  unsigned long mode = on ? 1 : 0;
  auto e = ::ioctlsocket(handle_, FIONBIO, &mode);
  r = e >= 0;
#endif
  return r;
}

int32_t ioctl(
    [[maybe_unused]] int32_t fd,
    [[maybe_unused]] int32_t request,
    [[maybe_unused]] void *argp) {
  int32_t r{-1};
#if OS_UNIX
  r = ::ioctl(fd, request, argp);
#elif OS_WIN
  r = ::ioctlex(fd, request, argp);
#endif
  return r;
}

uint32_t available([[maybe_unused]] int32_t fd) {
#if OS_UNIX
  uint32_t arg{0};
  ::ioctl(fd, FIONREAD, &arg);
  return arg;
#elif OS_WIN
  uint64_t arg{0};
  ::ioctlex(fd, FIONREAD, &arg);
  return static_cast<uint32_t>(arg);
#endif
}

int32_t dup(int32_t fd) {
  int32_t r{-1};
#if OS_UNIX
  r = ::dup(fd);
#elif OS_WIN
  r = ::_dup(fd);
#endif

  if (r < 0) {
#if OS_UNIX
    switch (errno) {
      case EBADF : 
      case EMFILE : 
      default : {
        break;
      }
    }
#elif OS_WIN
    //do nothing
#endif
  }
  return r;
}

int64_t lseek(int32_t fd, uint64_t offset, int32_t whence) {
#if OS_UNIX
  int64_t result = ::lseek(fd, offset, whence);
  if (result < 0) {
    switch (errno) {
      case EBADF : 
      case ESPIPE : 
      case EINVAL : 
      default : {
        break;
      }
    }
  }
#elif OS_WIN
  uint64_t result = ::_lseek(fd, (long)offset, whence);
  if ( result < 0 ) {
  }
#endif
  return result;
}

int64_t tell([[maybe_unused]] int32_t fd) {
  int64_t result = 0;
#if OS_UNIX
  //do nothing
#elif OS_WIN
  result = ::_tell(fd);
  if (result < 0) {
  }
#endif
  return result ;
}

bool truncate(const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (nullptr == fp) return false;
  fclose(fp);
  return true;
}

} // namespace plain
