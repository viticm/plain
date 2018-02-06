/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id global.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/05 23:08
 * @uses Base global variables.
*/
#ifndef PF_BASIC_GLOBAL_H_
#define PF_BASIC_GLOBAL_H_

#include "pf/basic/type/variable.h"

namespace pf_basic {

PF_API type::variable_set_t &get_globals();

}

#define GLOBALS pf_basic::get_globals()

#endif //PF_BASIC_GLOBAL_H_
