/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/21 14:15
 * @uses The net connection config.
 */

#ifndef PLAIN_NET_CONNECTION_CONFIG_H_
#define PLAIN_NET_CONNECTION_CONFIG_H_

#include "plain/net/config.h"

namespace plain::net {
namespace connection {

using id_t = int32_t;
using callable_func = std::function<void(Basic *)>;

static constexpr id_t kInvalidId{-1};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_CONFIG_H_
