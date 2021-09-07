/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id id.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/06 16:12
 * @uses your description
 */

#ifndef PF_EVENTS_ID_H_
#define PF_EVENTS_ID_H_

#include "pf/events/config.h"

namespace pf_events {

template <typename T>
struct type_id_ptr {
  static const T* const id;
};

template <typename T>
const T* const type_id_ptr<T>::id = nullptr;

using id_t = const void *;

template <typename T>
constexpr id_t get_id() { // Helper for getting "type id"
  return &type_id_ptr<T>::id;
}

template <class Event>
constexpr bool validate_event() {
  static_assert(std::is_const<Event>::value == false, 
      "Struct must be without const");
  static_assert(std::is_volatile<Event>::value == false, 
      "Struct must be without volatile");
  static_assert(std::is_reference<Event>::value == false, 
      "Struct must be without reference");
  static_assert(std::is_pointer<Event>::value == false, 
      "Struct must be without pointer");
  return true;
}

} // namespace pf_events

#endif // PF_EVENTS_ID_H_
