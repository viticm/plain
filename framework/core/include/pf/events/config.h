/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/08/13 17:49
 * @uses The events config file.
 *       Implement from: https://github.com/gelldur/EventBus.
 */

#ifndef PF_EVENTS_CONFIG_H_
#define PF_EVENTS_CONFIG_H_

#include "pf/basic/config.h"

namespace pf_events {

class Dispatcher;
class NoopStream;

class PostponeHelper;
class BusInterface;
class Bus;

template <typename>
class ListenerAttorney;

template <typename>
class Listener;

} // namespace pf_events

#endif // PF_EVENTS_CONFIG_H_
