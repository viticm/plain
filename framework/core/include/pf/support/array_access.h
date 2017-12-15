/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id array_access.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/14 14:39
 * @uses your description
*/
#ifndef PF_SUPPORT_ARRAY_ACCESS_H_
#define PF_SUPPORT_ARRAY_ACCESS_H_

#include "pf/support/config.h"

namespace pf_support {

class PF_API ArrayAccess {

 public:
   ArrayAccess() {};
   ~ArrayAccess() {};
 
 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_t = pf_basic::type::variable_t;

 public:

   //The array.
   variable_array_t items_;

 public:
   
   //Join array elements with a string.
   std::string implode(const std::string &pieces);

   //Get all of the items in the collection.
   variable_array_t all() {
     return items_;
   };

};

}; //namespace pf_support

#endif //PF_SUPPORT_ARRAY_ACCESS_H_
