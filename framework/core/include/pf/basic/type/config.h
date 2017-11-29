/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/05 20:28
 * @uses Base module type config.
*/
#ifndef PF_BASIC_TYPE_CONFIG_H_
#define PF_BASIC_TYPE_CONFIG_H_

#include "pf/basic/config.h"

namespace pf_basic {

namespace type {

struct variable_struct;

//Commonly used definitions.
using variable_t = variable_struct;
using variable_array_t = std::vector< variable_t >;
using variable_set_t = std::map< std::string, variable_t > ;

typedef enum {
  kVariableTypeInvalid = -1,
  kVariableTypeBool,
  kVariableTypeInt32,
  kVariableTypeUint32,
  kVariableTypeInt16,
  kVariableTypeUint16,
  kVariableTypeInt8,
  kVariableTypeUint8,
  kVariableTypeInt64,
  kVariableTypeUint64,
  kVariableTypeFloat,
  kVariableTypeDouble,
  kVariableTypeString,
  kVariableTypeNumber,
} var_t; //变量的类型

}; //namespace type

}; //namespace pf_basic

#endif //PF_BASIC_TYPE_CONFIG_H_
