/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/04 21:58
 * @uses The net socket config file.
 */

#ifndef PLAIN_NET_SOCKET_CONFIG_H_
#define PLAIN_NET_SOCKET_CONFIG_H_

#include "plain/net/config.h"

namespace plain::net {
namespace socket {

using id_t = int32_t;

static constexpr id_t kInvalidId{-1};
static constexpr int32_t kErrorWouldBlock{-100};
static constexpr int32_t kSocketError{-1};

} // namespace socket
} // namespace plain::net

#endif // PLAIN_NET_SOCKET_CONFIG_H_
