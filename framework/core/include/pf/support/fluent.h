/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id fluent.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/04 11:27
 * @uses your description
*/
#ifndef PF_SUPPORT_FLUENT_H_
#define PF_SUPPORT_FLUENT_H_

namespace pf_support {

class PF_API Fluent {

 public:
   Fluent() {}
   virtual ~Fluent() {}

 public:

   //Get an attribute from the container.
   variable_t get(const std::string &key, const variable_t &def);
   
 public:
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;

 protected:

   //All of the attributes set on the container.
   variable_set_t attributes_;

};

}; //namespace pf_support

#endif //PF_SUPPORT_FLUENT_H_
