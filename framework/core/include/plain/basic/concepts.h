/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id concepts.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/08 19:12
 * @uses The useful concepts.
 */

#ifndef PLAIN_BASIC_CONCEPTS_H_
#define PLAIN_BASIC_CONCEPTS_H_

#include "plain/basic/config.h"

namespace plain {

template <std::size_t N>
concept power_of_two = requires() {
  requires not(0 == N) and not(N & (N - 1));
};

template <typename T>
concept enums = std::is_enum_v<T>;

} // namespace plain

#endif // PLAIN_BASIC_CONCEPTS_H_
