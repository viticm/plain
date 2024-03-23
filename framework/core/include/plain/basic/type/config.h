/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 18:07
 * @uses The plain framework basic config header.
 */
#ifndef PLAIN_BASIC_TYPE_CONFIG_H_
#define PLAIN_BASIC_TYPE_CONFIG_H_

#include "plain/basic/config.h"
#include <unordered_map>

namespace plain {

struct variable_struct;

// Commonly used definitions.
// The variable_struct is a abstract data type(ADT).
using variable_t = variable_struct;
using variable_array_t = std::vector<variable_t>;
using variable_map_t = std::unordered_map<std::string, variable_t>;
using closure_t = std::function<void()>;
using bytes_t = std::basic_string<std::byte>;

} //namespace plain

#endif //PLAIN_BASIC_TYPE_CONFIG_H_
