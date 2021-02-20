/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id collection.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/14 16:18
 * @uses your description
*/
#ifndef PF_SUPPORT_COLLECTION_TCC_
#define PF_SUPPORT_COLLECTION_TCC_

#include "pf/support/collection.h"

namespace pf_support {

template <typename T>
inline ArrayAccess Collection<T>::map(map_callback_t callback) {
  ArrayAccess a;
  for (T &item : original_) 
    a.items_.emplace_back(callback(item));
  return a;
}

//Reduce the collection to a single value(string).
template <typename T>
inline std::string Collection<T>::reduce(reduce_callback_t callback, 
                                         const std::string &initial) {
  std::string r{initial};
  for (T &item : original_)
    callback(r, item);
  return r;
}

} //namespace pf_support

#endif //PF_SUPPORT_COLLECTION_TCC_
