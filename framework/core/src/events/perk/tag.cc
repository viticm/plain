#include "pf/events/perk/tag.h"

using namespace pf_events::perk;

flag_t Tag::on_pre_postpone_event(const PostponeHelper &call) {
  auto found = events_to_wrap_.find(call.id_);
  if (found != events_to_wrap_.end()) {
    found->second(call.event_);
    return flag_t::postpone_cancel;
  }
  return flag_t::postpone_continue;
}
