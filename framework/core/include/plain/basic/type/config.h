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

namespace plain {

struct variable_struct;

//Commonly used definitions.
using variable_t = variable_struct;
using variable_array_t = std::vector< variable_t >;
using variable_map_t = std::unordered_map< std::string, variable_t > ;
using closure_t = std::function<void()>;

enum class Variable {
  Invalid = -1,
  Bool,
  Int32,
  Uint32,
  Int16,
  Uint16,
  Int8,
  Uint8,
  Int64,
  Uint64,
  Float,
  Double,
  String,
  Number,
}; //变量的类型

} //namespace plain

#endif //PLAIN_BASIC_TYPE_CONFIG_H_
