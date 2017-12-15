/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id helpers.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/14 14:25
 * @uses your description
*/
#ifndef PF_SUPPORT_HELPERS_H_
#define PF_SUPPORT_HELPERS_H_

#include "pf/support/config.h"
#include "pf/support/collection.h"
#include "pf/basic/type/variable.h"

namespace pf_support {

//Create a collection from the given value.
template <typename T>
Collection<T> collect(const std::vector<T> &value) {
  Collection<T> o(value);
  return o;
}

//Concatenate values of a given key as a string.
inline std::string implode(const std::string &glue, 
    const pf_basic::type::variable_array_t & array) {
  std::string r{""};
  auto it = array.begin();
  for (;it != array.end(); ++it) {
    r += it != array.begin() && it != array.end() ? glue + (*it).data : (*it).data;
  }
  return r;
};

//Check the value is empty.
inline bool empty(const pf_basic::type::variable_t &value) {
  return pf_basic::type::kVariableTypeInvalid == value.type;
}

};

#endif //PF_SUPPORT_HELPERS_H_
