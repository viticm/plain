/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id listener_traits.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/06 10:50
 * @uses The event listener traits.
 */

#ifndef PF_EVENTS_LISTENER_TRAITS_H_
#define PF_EVENTS_LISTENER_TRAITS_H_

#include "pf/events/config.h"

namespace pf_events {

template <typename Ret, typename Arg, typename... Rest>
Arg first_argument_helper(Ret (*)(Arg, Rest...));

template <typename Ret, typename F, typename Arg, typename... Rest>
Arg first_argument_helper(Ret (F::*)(Arg, Rest...));

template <typename Ret, typename F, typename Arg, typename... Rest>
Arg first_argument_helper(Ret (F::*)(Arg, Rest...) const);

template <typename F>
decltype(first_argument_helper(&F::operator())) first_argument_helper(F); 

template <typename T> 
using first_argument = decltype(first_argument_helper(std::declval<T>()));

} // namespace pf_events

#endif // PF_EVENTS_LISTENER_TRAITS_H_
