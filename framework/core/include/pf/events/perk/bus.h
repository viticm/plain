/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id bus.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/15 10:51
 * @uses The perk event bus class.
 */

#ifndef PF_EVENTS_PERK_BUS_H_
#define PF_EVENTS_PERK_BUS_H_

#include "pf/events/perk/config.h"
#include "pf/events/bus.h"

namespace pf_events {

namespace perk {

class PF_API Bus : public pf_events::Bus {

 public:

   class RegisterHelper {
    
    public:

      friend class perk::Bus;

    public:

      template <typename perk_t>
      RegisterHelper& register_pre_postpone(
          flag_t (perk_t::*method)(PostponeHelper &)) {
        bus_->on_pre_postpone_.push_back(
            std::bind(method, 
              static_cast<perk_t*>(perk_), std::placeholders::_1));
        return *this;
      }

      template <typename perk_t>
      RegisterHelper& register_post_postpone(
          flag_t (perk_t::*method)(PostponeHelper &)) {
        bus_->on_post_postpone_.push_back(
            std::bind(method, 
              static_cast<perk_t*>(perk_), std::placeholders::_1));
        return *this;
      }

    private:

      perk::Bus *bus_;
      Basic *perk_;

    private:

      RegisterHelper(perk::Bus *bus, Basic *perk) 
        : bus_{bus}, perk_{perk}
      {}

   };

 public:

   RegisterHelper add_perk(std::shared_ptr<Basic> perk);
   
   template <typename T>
   T* get_perk() {
     auto found = std::find_if(perks_.begin(), perks_.end(),
         [](const std::shared_ptr<Basic> &perk) { 
       return dynamic_cast<T*>(perk.get()) != nullptr;
     });
     if (found != perks_.end()) {
       return static_cast<T*>(found->get());
     }
     return nullptr;
   }

 protected:

   bool postpone_event(const PostponeHelper &call) override;

 private:

   std::vector< std::shared_ptr<Basic> > perks_;
   std::vector< std::function<perk::flag_t(const PostponeHelper &)> > 
     on_pre_postpone_;
   std::vector< std::function<perk::flag_t(const PostponeHelper &)> > 
     on_post_postpone_;

};

} // namespace perk

} // namespace pf_events

#endif // PF_EVENTS_PERK_BUS_H_
