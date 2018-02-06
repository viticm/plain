/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id expression.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/11/29 20:56
 * @uses your description
*/
#ifndef PF_DB_QUERY_EXPRESSION_H_
#define PF_DB_QUERY_EXPRESSION_H_

#include "pf/db/query/config.h"

namespace pf_db {

class Expression {

 public:
   Expression(const std::string &value) : value_{value} {};
   ~Expression() {};

 public:

   //Get the value of the expression.
   const std::string get_value() const { return value_; };

 protected:

   //The value of the expression.
   std::string value_;

};

} //namespace pf_db

#endif //PF_DB_QUERY_EXPRESSION_H_
