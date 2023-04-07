/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id constants.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/07 18:35
 * @uses The constants define of basic.
 */

#ifndef PLAIN_BASIC_CONSTANTS_H_
#define PLAIN_BASIC_CONSTANTS_H_

#include "plain/basic/config.h"

namespace plain::detail::consts {

constexpr int32_t kSmallBufferSize{4096};
constexpr int32_t kLargeBufferSize{4096 * 1024};

} // namespace plain::detail::consts

#endif // PLAIN_BASIC_CONSTANTS_H_
