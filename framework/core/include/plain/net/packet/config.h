/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/20 18:21
 * @uses The net packet config.
 */

#ifndef PLAIN_NET_PACKET_CONFIG_H_
#define PLAIN_NET_PACKET_CONFIG_H_

#include "plain/net/config.h"

namespace plain::net {
namespace packet {

using dispatch_func = std::function<
  bool(connection::Basic *, std::shared_ptr<Basic>)>;
using id_t = uint16_t;

static constexpr id_t kMaxId{std::numeric_limits<id_t>::max()};
static constexpr id_t kRpcRequestId{kMaxId - 1};
static constexpr id_t kRpcResponseId{kMaxId - 2};
static constexpr id_t kRpcNotifyId{kMaxId - 3};

} // namespace packet
} // namespace plain::net

#endif // PLAIN_NET_PACKET_CONFIG_H_
