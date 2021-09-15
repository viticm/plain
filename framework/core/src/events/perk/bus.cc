#include "pf/events/perk/bus.h"

using namespace pf_events::perk;

Bus::RegisterHelper Bus::add_perk(std::shared_ptr<Basic> perk) {
  auto *local = perk.get();
  perks_.push_back(std::move(perk));
  return RegisterHelper(this, local);
}

bool Bus::postpone_event(const PostponeHelper &call) {
  for (const auto &on_pre_postpone : on_pre_postpone_) {
    if (on_pre_postpone(call) == flag_t::postpone_cancel)
      return false;
  }
  if (pf_events::Bus::postpone_event(call)) {
    for (const auto &on_post_postpone : on_post_postpone_) {
      if(on_post_postpone(call) == flag_t::postpone_cancel) {
        break;
      }
    }
    return true;
  }
  return false;
}
