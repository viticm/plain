/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id api.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/08 14:09
 * @uses The socket api(system functions) implemention.
 */

#ifndef PLAIN_NET_SOCKET_API_H_
#define PLAIN_NET_SOCKET_API_H_

#include "plain/net/socket/config.h"
#if OS_WIN
#include <winsock.h>
#elif OS_UNIX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include "plain/basic/error.h"

namespace plain::net {
namespace socket {

PLAIN_API bool initialize();

PLAIN_API id_t create(int32_t domain, int32_t type, int32_t protocol);

PLAIN_API bool
bind(id_t id, const sockaddr *addr, uint32_t addrlength);

PLAIN_API bool
connect(
  id_t id, const sockaddr *name, uint32_t namelength,
  const std::chrono::milliseconds &timeout = {});

PLAIN_API bool listen(id_t id, uint32_t backlog);

PLAIN_API int32_t 
accept(id_t id, sockaddr *addr, uint32_t *addrlength);

PLAIN_API bool
getsockoptb(
  id_t id, int32_t level, int32_t optname, void *optval, uint32_t *optlength);

PLAIN_API uint32_t
getsockoptu(
  id_t id, int32_t level, int32_t optname, void *optval, uint32_t *optlength);

PLAIN_API bool
setsockopt(
  id_t id, int32_t level, int32_t optname, const void *optval,
  uint32_t optlength);

PLAIN_API int32_t
send(id_t id, const void *buffer, uint32_t length, uint32_t flag);

PLAIN_API int32_t
sendto(
  id_t id, const void *buffer, int32_t length, uint32_t flag,
  const sockaddr *to, int32_t tolength);

PLAIN_API int32_t
recv(id_t id, void *buffer, uint32_t length, uint32_t flag);

PLAIN_API int32_t
recvfrom(
  id_t id, void *buffer, int32_t length, uint32_t flag,
  sockaddr *from, uint32_t *fromlength);

PLAIN_API bool close(id_t id);

PLAIN_API bool ioctl(id_t id, int64_t cmd, uint64_t *argp);

PLAIN_API bool get_nonblocking(id_t id);

PLAIN_API bool set_nonblocking(id_t id, bool on);

PLAIN_API uint32_t available(id_t id);

PLAIN_API bool shutdown(id_t id, int32_t how);

PLAIN_API int32_t
select(
  id_t maxfdp, fd_set *readset, fd_set *writeset, fd_set *exceptset,
  timeval *timeout);

PLAIN_API int32_t
getsockname(id_t id, sockaddr *name, int32_t *namelength);

PLAIN_API int32_t
getpeername(id_t id, sockaddr *name, int32_t *namelength);

PLAIN_API Error get_last_error() noexcept;

PLAIN_API bool make_pair(id_t fd_pair[2]) noexcept;

} // namespace socket
} // namespace plain::net

#endif // PLAIN_NET_SOCKET_API_H_
