/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id net.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/02/27 17:49
 * @uses The net utility functions.
 */

#ifndef NET_H_
#define NET_H_

#include "plain/net/config.h"
#include "plain/concurrency/config.h"
#include "plain/engine/config.h"

[[nodiscard]] plain::Timer
counter_net(
  std::shared_ptr<plain::net::Listener> listener,
  std::shared_ptr<plain::concurrency::executor::Basic> executor = {}) noexcept;

[[nodiscard]] plain::Timer
counter_net(
  std::shared_ptr<plain::net::Connector> connector,
  std::shared_ptr<plain::concurrency::executor::Basic> executor = {}) noexcept;

void wait_shutdown(std::shared_ptr<plain::net::Listener> listener) noexcept;
void wait_shutdown(std::shared_ptr<plain::net::Connector> connector) noexcept;

#endif // NET_H_
