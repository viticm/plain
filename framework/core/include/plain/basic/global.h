/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id global.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/03/31 18:44
 * @uses Plain global variables.
*/
#ifndef PLAIN_BASIC_GLOBAL_H_
#define PLAIN_BASIC_GLOBAL_H_

#include "plain/basic/type/variable.h"

namespace plain {

PLAIN_API variable_map_t &get_globals();

} // namespace plain

#define GLOBALS plain::get_globals()

#endif //PLAIN_BASIC_GLOBAL_H_
