#include "plain/net/socket/api.h"
#include <errno.h>
#if OS_UNIX || OS_MAC
#include <signal.h>
#endif
#include "plain/file/api.h"
#include "plain/basic/type/config.h"
#include "plain/sys/utility.h"

namespace plain::net::socket {

static thread_local Error s_error;

static void set_error(int32_t e) {
  s_error.set_code(e);
#if OS_WIN
  switch (s_error.code()) {
      case WSANOTINITIALISED : {
        s_error.set_message("WSANOTINITIALISED");
        break;
      }
      case WSAENETDOWN : {
        s_error.set_message("WSAENETDOWN");
        break;
      }
      case WSAEFAULT : {
        s_error.set_message("WSAEFAULT");
        break;
      }
      case WSAENOTCONN : {
        s_error.set_message("WSAENOTCONN");
        break;
      }
      case WSAEINTR : {
        s_error.set_message("WSAEINTR");
        break;
      }
      case WSAEINPROGRESS : {
        s_error.set_message("WSAEINPROGRESS");
        break;
      }
      case WSAENETRESET : {
        s_error.set_message("WSAENETRESET");
        break;
      }
      case WSAENOTSOCK : {
        s_error.set_message("WSAENOTSOCK");
        break;
      }
      case WSAEOPNOTSUPP : {
        s_error.set_message("WSAEOPNOTSUPP");
        break;
      }
      case WSAESHUTDOWN: {
        s_error.set_message("WSAESHUTDOWN");
        break;
      }
      case WSAEWOULDBLOCK : {
        s_error.set_code(kErrorWouldBlock);
        s_error.set_message("WSAEWOULDBLOCK");
        break;
      }
      case WSAEMSGSIZE : {
        s_error.set_message("WSAEMSGSIZE");
        break;
      }
      case WSAEINVAL : {
        s_error.set_message("WSAEINVAL");
        break;
      }
      case WSAECONNABORTED : {
        s_error.set_message("WSAECONNABORTED");
        break;
      }
      case WSAETIMEDOUT : {
        s_error.set_message("WSAETIMEDOUT");
        break;
      }
      case WSAECONNRESET : {
        s_error.set_message("WSAECONNRESET");
        break;
      }
      default : {
        s_error.set_message("UNKNOWN");
        break;
      }
    }
#else
  switch (s_error.code()) {
    case EWOULDBLOCK : {
      s_error.set_code(kErrorWouldBlock);
      s_error.set_message("EWOULDBLOCK");
      break;
    }
    case ECONNRESET : {
      s_error.set_message("ECONNRESET");
      break;
    }
    case ECONNABORTED : {
      s_error.set_message("ECONNABORTED");
      break;
    }
    case EPROTO : {
      s_error.set_message("EPROTO");
      break;
    }
    case EINTR : {
      // from UNIX Network Programming 2nd, 15.6
      // with nonblocking-socket, ignore above errors
      s_error.set_message("EINTR");
      break;
    }
    case EBADF : {
      s_error.set_message("EBADF");
      break;
    }
    case ENOTSOCK : {
      s_error.set_message("ENOTSOCK");
      break;
    }
    case EOPNOTSUPP : {
      s_error.set_message("EOPNOTSUPP");
      break;
    }
    case EFAULT : {
      s_error.set_message("EFAULT");
      break;
    }
    case ETIMEDOUT: {
      s_error.set_message("ETIMEDOUT");
      break;
    }
    default : {
      break;
    }
  }
#endif
}

static void set_error() {
  int32_t e{-1};
#if OS_WIN
  e = WSAGetLastError();
#else
  e = errno;
#endif
  set_error(e);
}

class Initializer : noncopyable {

 public:
  Initializer();
  ~Initializer();

 public:

  bool success() const noexcept {
    return success_;
  }

  static bool init() noexcept {
    static Initializer s_init;
    return s_init.success();
  }

 private:
  bool success_{true};

};

Initializer::Initializer() {
#if OS_WIN
  WSADATA data;
  auto e = ::WSAStartup(MAKEWORD(2, 0), &data);
  success_ = 0 == e;
#elif OS_UNIX || OS_MAC
  ::signal(SIGPIPE, SIG_IGN);
  if (!set_core_rlimit())
    success_ = false;
#endif
  if (!success_) set_error();
}

Initializer::~Initializer() {
#if OS_WIN
  ::WSACleanup();
#endif
}

bool initialize() {
  return Initializer::init();
}

id_t create(int32_t domain, int32_t type, int32_t protocol) {
  id_t id = static_cast<id_t>(::socket(domain, type, protocol));
  if (id == kInvalidId) {
    set_error();
  }
  return id;
}

bool bind(id_t id, const sockaddr *addr, uint32_t addrlength) {
  auto e = ::bind(id, addr, addrlength);
  bool r = e != kSocketError;
  if (!r) set_error();
  return r;
}

bool connect(
  id_t id, const sockaddr *addr, uint32_t addrlength,
  const std::chrono::milliseconds &timeout) {
  auto nonblocking = get_nonblocking(id);
  if (!nonblocking && !set_nonblocking(id, true)) {
    set_error();
    return false;
  }
  auto e = ::connect(id, addr, addrlength);
#if OS_WIN
  const auto connect_errno = WSAGetLastError();
#elif OS_UNIX || OS_MAC
  const auto connect_errno = errno;
#else
  const auto connect_errno = 0;
#endif
  bool r = e != kSocketError;
  if (r) return r;
  if (!nonblocking && !set_nonblocking(id, false)) {
    set_error();
    return false;
  }
#if OS_WIN
  if (connect_errno != WSAEINPROGRESS && connect_errno != WSAEWOULDBLOCK) {
    set_error();
    return false;
  }
#elif OS_UNIX || OS_MAC
  if (connect_errno != EINPROGRESS) {
    set_error();
    return false;
  }
#endif
  auto read_fds = fd_set{};
  FD_ZERO(&read_fds);
  FD_SET(id, &read_fds);
  auto write_fds = fd_set{};
  FD_ZERO(&write_fds);
  FD_SET(id, &write_fds);
  if (timeout.count() > 0) {
    timeval tv{
      static_cast<decltype(tv.tv_sec)>(timeout.count() / 1000),
      static_cast<decltype(tv.tv_usec)>(timeout.count() % 1000 * 1000)
    };
    e = socket::select(FD_SETSIZE, &read_fds, &write_fds, nullptr, &tv);
  } else {
    e = socket::select(FD_SETSIZE, &read_fds, &write_fds, nullptr, nullptr); 
  }
  r = (e > 0);
  if (!r) {
    // zero return is mean timeout.
    if (e == 0) {
#if OS_WIN
      e = WSAETIMEDOUT;
#else
      e = ETIMEDOUT;
#endif
    }
    set_error(e);
    return false;
  }
  uint32_t length{static_cast<uint32_t>(sizeof(e))};
  getsockoptb(id, SOL_SOCKET, SO_ERROR, &e, &length);
  r = (e == 0);
  if (!r) set_error(e);
  return r;
}

bool listen(id_t id, uint32_t backlog) {
  auto e = ::listen(id, backlog);
  bool r = e != kSocketError;
  if (!r) set_error();
  return r;
}

int32_t accept(id_t id, sockaddr *addr, uint32_t *addrlength) {
  int32_t r{kSocketError};
#if OS_UNIX || OS_MAC
  r = ::accept(id, addr, addrlength);
#elif OS_WIN
  r = static_cast<int32_t>(
    ::accept(id, addr, reinterpret_cast<int32_t *>(addrlength)));
#endif
  if (r == kSocketError) set_error();
  return r;
}

bool getsockoptb(
  id_t id, int32_t level, int32_t optname, void *optval, uint32_t *optlength) {
  int32_t e{kSocketError};
#if OS_UNIX || OS_MAC
  e = ::getsockopt(id, level, optname, optval, optlength);
#elif OS_WIN
  e = ::getsockopt(
    id, level, optname, static_cast<char *>(optval),
    reinterpret_cast<int32_t *>(optlength));
#endif
  bool r = e != kSocketError;
  if (!r) set_error();
  return r;
}

uint32_t getsockoptu(
  id_t id, int32_t level, int32_t optname, void *optval, uint32_t *optlength) {
  uint32_t r{0};
#if OS_UNIX || OS_MAC
  if (::getsockopt(id, level, optname, optval, optlength) == kSocketError) {
    set_error();
    switch (s_error.code()) {
      case EBADF:
        r = 1;
        break;
      case ENOTSOCK:
        r = 2;
        break;
      case ENOPROTOOPT:
        r = 3;
        break;
      case EFAULT:
        r = 4;
        break;
      default:
        r = 5;
        break;
    }
  }
#elif OS_WIN
  auto e = ::getsockopt(
    id, level, optname, static_cast<char *>(optval),
    reinterpret_cast<int32_t *>(optlength));
  if (e == kSocketError) set_error();
#endif
  return r;
}

bool setsockopt(
  id_t id, int32_t level, int32_t optname, const void *optval,
  uint32_t optlength) {
  int32_t e{kSocketError};
#if OS_UNIX || OS_MAC
  e = ::setsockopt(id, level, optname, optval, optlength);
#elif OS_WIN
  e = ::setsockopt(id, level, optname, reinterpret_cast<const char *>(optval), optlength);
#endif
  bool r = e != kSocketError;
  if (!r) set_error();
  return r;
}

int32_t send(id_t id, const void *buffer, uint32_t length, uint32_t flag) {
  int32_t r{kSocketError};
#if OS_UNIX || OS_MAC
  r = ::send(id, buffer, length, flag);
#elif OS_WIN
  r = ::send(id, static_cast<const char *>(buffer), length, flag);
#endif
  if (r == kSocketError) {
    set_error();
    if (s_error.code() == kErrorWouldBlock) r = kErrorWouldBlock;
  }
  return r;
}

int32_t sendto(
  id_t id, const void *buffer, int32_t length, uint32_t flag,
  const sockaddr *to, int32_t tolength) {
  int32_t r{kSocketError};
#if OS_UNIX || OS_MAC
  r = ::sendto(id, buffer, length, flag, to, tolength);
#elif OS_WIN
  r = ::sendto(id, static_cast<const char *>(buffer),length, flag, to, tolength);
#endif
  if (r == kSocketError) {
    set_error();
    if (s_error.code() == kErrorWouldBlock) r = 0;
  }
  return r;
}

int32_t recv(id_t id, void *buffer, uint32_t length, uint32_t flag) {
  int32_t r{kSocketError};
#if OS_UNIX || OS_MAC
  r = ::recv(id, buffer, length, flag);
#elif OS_WIN
  r = ::recv(id, static_cast<char *>(buffer), length, flag);
#endif
  // std::cout << "id: " << id << "|" << length << "|" << r << std::endl;
  if (r == kSocketError) {
    set_error();
    if (s_error.code() == kErrorWouldBlock) r = kErrorWouldBlock;
  } 
  return r;
}

int32_t recvfrom(
  id_t id, void *buffer, int32_t length, uint32_t flag, sockaddr *from,
  uint32_t *fromlength) {
  int32_t r{kSocketError};
#if OS_UNIX || OS_MAC
  r = ::recvfrom(id, buffer, length, flag, from, fromlength);
#elif OS_WIN
  r = ::recvfrom(
    id, static_cast<char *>(buffer), length, flag, from,
    reinterpret_cast<int32_t *>(fromlength));
#endif
  if (r == kSocketError) {
    set_error();
    if (s_error.code() == kErrorWouldBlock) r = kErrorWouldBlock;
  }
  return r;
}

bool close(id_t id) {
  int32_t e{kSocketError};
#if OS_UNIX || OS_MAC
  e = ::close(id);
#elif OS_WIN
  e = ::closesocket(id);
#endif
  bool r = e != kSocketError;
  if (!r) set_error();
  return r;
}

bool ioctl(
  [[maybe_unused]] id_t id, [[maybe_unused]] int64_t cmd,
  [[maybe_unused]] uint64_t *argp) {
  bool r{false};
#if OS_WIN
  auto e = ioctlsocket(id, static_cast<long>(cmd), reinterpret_cast<u_long *>(argp));
  r = e != kSocketError;
  if (!r) set_error();
#endif
  return r;
}

bool get_nonblocking(id_t id) {
  return plain::get_nonblocking(id);
}

bool set_nonblocking(id_t id, bool on) {
  auto r = plain::set_nonblocking(id, on);
  if (!r) set_error();
  return r;
}

uint32_t available(id_t id) {
  return plain::available(id);
}

bool shutdown(id_t id, int32_t how) {
  bool r{false};
  auto e = ::shutdown(id, how);
  r = e != kSocketError;
  if (!r) set_error();
  return r;
}

int32_t select(
  id_t maxfdp, fd_set *readset, fd_set *writeset, fd_set *exceptset,
  timeval *timeout) {
  auto r = ::select(maxfdp, readset, writeset, exceptset, timeout);
  if (r == kSocketError) set_error();
  return r;
}

int32_t getsockname(id_t id, sockaddr *name, int32_t *namelength) {
  int32_t r{0};
#if OS_UNIX || OS_MAC
  r = ::getsockname(id, name, reinterpret_cast<socklen_t *>(namelength));
#elif OS_WIN
  r = ::getsockname(id, name, namelength);
#endif
  if (r == kSocketError) set_error();
  return r;
}

int32_t getpeername(id_t id, sockaddr *name, int32_t *namelength) {
  int32_t r{0};
#if OS_UNIX || OS_MAC
  r = ::getpeername(id, name, reinterpret_cast<socklen_t *>(namelength));
#elif OS_WIN
  r = ::getpeername(id, name, namelength);
#endif
  if (r == kSocketError) set_error();
  return r;
}

Error get_last_error() noexcept {
  return s_error;
}

int32_t socketpair(
  int32_t family, int32_t type, int32_t protocol, id_t fds[2]) {
#if OS_WIN
  id_t tcp1{kInvalidId}, tcp2{ kInvalidId };
  bytes_t name;
  if (family == AF_INET6) {
    sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = static_cast<ADDRESS_FAMILY>(family);
    auto cr = inet_pton(AF_INET6, "::1", &addr.sin6_addr);
    if (cr != 1) return -1;
    name.append(reinterpret_cast<std::byte *>(&addr), sizeof(addr));
  } else {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = static_cast<ADDRESS_FAMILY>(family);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    name.append(reinterpret_cast<std::byte *>(&addr), sizeof(addr));
  }

  int32_t namelen = static_cast<int32_t>(name.size());
  id_t tcp = create(family, type, protocol);
  if (tcp == kInvalidId){
    goto clean;
  }
  if (::bind(tcp, reinterpret_cast<sockaddr *>(name.data()), namelen) == -1){
    goto clean;
  }
  if (!listen(tcp, 5)){
    goto clean;
  }
  if (::getsockname(
      tcp, reinterpret_cast<sockaddr *>(name.data()), &namelen) == -1){
    goto clean;
  }
  tcp1 = create(family, type, protocol);
  if (tcp1 == kInvalidId){
    goto clean;
  }
  if (kSocketError == ::connect(
      tcp1, reinterpret_cast<sockaddr *>(name.data()), namelen)){
    goto clean;
  }

  tcp2 = static_cast<id_t>(
    ::accept(tcp, reinterpret_cast<sockaddr *>(name.data()), &namelen));
  if (tcp2 == kInvalidId){
    goto clean;
  }
  if (!close(tcp)){
    goto clean;
  }
  fds[0] = tcp1;
  fds[1] = tcp2;
  return 0;
clean:
  if (tcp != kInvalidId){
    close(tcp);
  }
  if (tcp2 != kInvalidId){
    close(tcp2);
  }
  if (tcp1 != kInvalidId){
    close(tcp1);
  }
  return kInvalidId;
#else
  return ::socketpair(family, type, protocol, fds);
#endif
}

bool make_pair(id_t fd_pair[2]) noexcept {
#if OS_WIN
  auto r = socket::socketpair(AF_INET, SOCK_STREAM, 0, fd_pair);
#elif OS_MAC
  auto r = ::socketpair(AF_UNIX, SOCK_STREAM, 0, fd_pair);
#else
  auto r = ::socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fd_pair);
#endif
  return r == 0;
}

int32_t get_sock_type(Type sock_type) noexcept {
  int32_t r{SOCK_STREAM};
  switch (sock_type) {
    case Type::Tcp:
      r = SOCK_STREAM;
      break;
    case Type::Udp:
      r = SOCK_DGRAM;
      break;
    default:
      break;
  }
  return r;
}

} // namespace plain::net::socket
