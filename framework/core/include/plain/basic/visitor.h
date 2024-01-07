/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id visitor.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/12/04 11:51
 * @uses The visitor helper implemention.
 */

#ifndef PLAIN_BASIC_VISITOR_H_
#define PLAIN_BASIC_VISITOR_H_

#include "plain/basic/config.h"

namespace plain {

// helper type for the visitor
template<typename ...T>
struct overloaded : T... { using T::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<typename ...T>
overloaded(T...) -> overloaded<T...>;

} // namespace plain

#endif // PLAIN_BASIC_VISITOR_H_
