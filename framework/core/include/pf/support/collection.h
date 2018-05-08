/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id collection.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/14 14:31
 * @uses your description
*/
#ifndef PF_SUPPORT_COLLECTION_H_
#define PF_SUPPORT_COLLECTION_H_

#include "pf/support/config.h"
#include "pf/support/array_access.h"

namespace pf_support {

template <typename T>
class Collection {

 public:
   Collection(const std::vector<T> &original) : original_{original} {};
   ~Collection() {};

 public:

   //The currnt array.
   std::vector<T> original_;

 public:

   //The map loop touch on item.
   using map_callback_t = std::function<std::string (T &)>;

   //The filter callback function.
   using filter_callback_t = std::function<bool (T &)>;

   //The reduce callback function.
   using reduce_callback_t = std::function<std::string (std::string &, T &)>;

 public:

   //Run a map over each of the items.
   ArrayAccess map(map_callback_t callback);

   //Create a collection of all elements that do not pass a given truth test.
   ArrayAccess reject(filter_callback_t callback);

   //Reduce the collection to a single value(string).
   std::string reduce(reduce_callback_t callback, 
                      const std::string &initial = "");


};

} //namespace pf_support

#include "pf/support/collection.tcc"

#endif //PF_SUPPORT_COLLECTION_H_
