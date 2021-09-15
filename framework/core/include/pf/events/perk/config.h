/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/15 10:21
 * @uses The event perk config file.
 */

#ifndef PF_EVENTS_PERK_CONFIG_H_
#define PF_EVENTS_PERK_CONFIG_H_

#include "pf/events/config.h"

namespace pf_events {

namespace perk {

enum class flag_t : int {
  nop,
  postpone_cancel,
  postpone_continue,
};

class Basic;
class PassEverything;
class Bus;

} // namespace perk

} // namespace pf_events

#endif // PF_EVENTS_PERK_CONFIG_H_
