#include "pf/events/perk/pass_everything.h"

using namespace pf_events::perk;

flag_t PassEverything::on_pre_postpone_event(const PostponeHelper &call) {
  call.postpone_callback_(*pass_to_bus_, std::move(call.event_));
  return flag_t::postpone_cancel;
}
