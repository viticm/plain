/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id grammar.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/11/29 19:56
 * @uses The contract class of database garmmar.
*/
#ifndef PF_DB_GRAMMAR_H_
#define PF_DB_GRAMMAR_H_

#include "pf/db/config.h"
#include "pf/basic/type/variable.h"

namespace pf_db {

class PF_API Grammar {

 public:
   Grammar() : table_prefix_{""} {};
   virtual ~Grammar() {};

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;

 public:
   
   //Set the grammar's table prefix.
   void set_table_prefix(const std::string &prefix) {
     table_prefix_ = prefix;
   };

   //Get the grammar's table prefix.
   const std::string get_table_prefix() const {
      return table_prefix_; 
   };

   //Wrap an array of values.
   void wrap_array(variable_array_t &values);

   //Wrap a table in keyword identifiers.
   virtual std::string wrap_table(const variable_t &table) {
     if (!is_expression(table)) {
       return wrap(table_prefix_ + table.data, true);
     }
     return table.data;
   };

   //Wrap a value in keyword identifiers.
   std::string wrap(const variable_t &value, bool prefix_alias = false);

   //Wrap a value that has an alias.
   std::string wrap_aliased_value(
       const variable_t &value, bool prefix_alias = false);

   //Wrap the given value segments.
   std::string wrap_segments(const variable_array_t &segments);

   //Wrap a single string in keyword identifiers.
   virtual std::string wrap_value(const variable_t &value);

   //Convert an array of column names into a delimited string.
   std::string columnize(const std::vector<std::string> &columns);

   //Create query parameter place-holders for an array.
   std::string parameterize(variable_set_t &values);

   //Get the appropriate query parameter place-holder for a value.
   std::string parameter(const variable_t &value) {
     return is_expression(value) ? get_value(value) : "?";
   };

   //Get the value of a raw expression.
   std::string get_value(const variable_t &value) {
     if (!is_expression(value)) return "";
     return DB_EXPRESSION_TYPE == value.type ? value.data : 
            "\"" + value.data + "\"";
   }

 public:
   
   //Get the format for database stored dates.
   virtual std::string get_date_format() {
     return "Y-m-d H:i:s";
   }

 protected:

   //The grammar table prefix.
   std::string table_prefix_;

};


} //namespace pf_db

#endif //PF_DB_GRAMMAR_H_
