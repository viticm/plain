/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id socket.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/09/28 18:25
 * @uses The socket detail implemention.
 */

#ifndef PLAIN_NET_DETAIL_SOCKET_H_
#define PLAIN_NET_DETAIL_SOCKET_H_

#include "plain/net/detail/config.h"
#include <optional>
#if OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "plain/basic/type/config.h"

namespace plain::net {
namespace detail {

std::string to_string(const in_addr &addr) noexcept;

std::string to_string(const in6_addr &addr) noexcept;

const sockaddr *get_sa_pointer(const bytes_t &addr) noexcept;

sockaddr *get_sa_pointer(bytes_t &addr) noexcept;

const sockaddr_in *get_sin_pointer(const bytes_t &addr) noexcept;
sockaddr_in *get_sin_pointer(bytes_t &addr) noexcept;

const sockaddr_in6 *get_sin6_pointer(const bytes_t &addr) noexcept;

sockaddr_in6 *get_sin6_pointer(bytes_t &addr) noexcept;

#if !OS_WIN
const sockaddr_un *get_sun_pointer(const bytes_t &addr) noexcept;

sockaddr_un *get_sun_pointer(bytes_t &addr) noexcept;

const char *get_path_pointer(const sockaddr_un *sun) noexcept;

char *get_path_pointer(sockaddr_un *sun) noexcept;

const char *get_path_pointer(const bytes_t &addr) noexcept;

char *get_path_pointer(bytes_t &addr) noexcept;
#endif

int32_t get_sa_family(const bytes_t &addr) noexcept;

size_t get_sa_size_minimum(int32_t family) noexcept;

size_t get_sa_size_maximum(int32_t family) noexcept;

uint16_t get_sin_port(const bytes_t &addr) noexcept;

uint16_t get_sin6_port(const bytes_t &addr) noexcept;

bytes_t get_sa_data(const bytes_t &addr) noexcept;

in_addr 
get_sin_addr(const bytes_t &addr, const in_addr &sin_addr = {}) noexcept;

in6_addr
get_sin6_addr(const bytes_t &addr, const in6_addr &sin6_addr = {}) noexcept;

std::optional<std::string>
to_path([[maybe_unused]] const bytes_t &addr) noexcept;


} // namespace detail
} // namespace plain::net

#endif // PLAIN_NET_DETAIL_SOCKET_H_
