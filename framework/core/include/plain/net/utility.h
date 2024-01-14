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

std::shared_ptr<connection::Manager>
make_manager(Mode mode, const setting_t &setting = {}) noexcept;

std::shared_ptr<connection::Manager>
make_manager(
  Mode mode, std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting = {}) noexcept;

} // namespace plain::net

#endif // PLAIN_NET_UTILITY_H_
