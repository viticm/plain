/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id utility.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/09 17:39
 * @uses The utility for net implemention.
 */

#ifndef PLAIN_NET_UTILITY_H_
#define PLAIN_NET_UTILITY_H_

#include "plain/net/config.h"
#include "plain/concurrency/config.h"

namespace plain::net {

PLAIN_API std::shared_ptr<connection::Manager>
make_manager(
  const setting_t &setting = {},
  std::shared_ptr<concurrency::executor::Basic> executor = {}) noexcept;

} // namespace plain::net

#endif // PLAIN_NET_UTILITY_H_
