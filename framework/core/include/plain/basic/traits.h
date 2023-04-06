/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id traits.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/06 11:02
 * @uses The extend traits.
 */

#ifndef PLAIN_BASIC_TRAITS_H_
#define PLAIN_BASIC_TRAITS_H_

#include <type_traits>

namespace plain {

template<typename E>
constexpr auto
to_underlying_t(E enumerator) noexcept {
  return static_cast<std::underlying_type_t<E>>(enumerator);
}

} // namespace plain

#endif // PLAIN_BASIC_TRAITS_H_
