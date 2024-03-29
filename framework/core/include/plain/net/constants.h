/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id constants.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/03/29 09:37
 * @uses The constants of net.
 *       ! Constants files can not include the module config file(config file 
 *        could include this file).
 */

#ifndef PLAIN_NET_CONSTANTS_H_
#define PLAIN_NET_CONSTANTS_H_

#include "plain/basic/config.h"

namespace plain::net {

enum class Mode : std::uint8_t {
  Select = 0,
  Epoll = 1,
  IoUring = 2,
  Iocp = 3,
  Kqueue = 4,
};

#if OS_WIN
constexpr Mode kOptimalMode{Mode::Iocp};
#elif OS_UNIX
constexpr Mode kOptimalMode{Mode::Epoll};
#elif OS_MAC
constexpr Mode kOptimalMode{Mode::Kqueue};
#else
constexpr Mode kOptimalMode{Mode::Select};
#endif

constexpr uint32_t kPacketIdMax{std::numeric_limits<uint16_t>::max()};
constexpr uint32_t kPacketLengthMax{200 * 1024};

constexpr uint32_t kConnectionCountMax{1024};
constexpr uint32_t kConnectionCountDefault{32};

} // namespace plain::net

#endif // PLAIN_NET_CONSTANTS_H_
