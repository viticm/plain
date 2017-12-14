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

namespace pf_support {

//Create a collection from the given value.
template <typename T>
Collection<T> collect(const std::vector<T> &value) {
  Collection<T> o(value);
  return o;
}

};

#endif //PF_SUPPORT_HELPERS_H_
