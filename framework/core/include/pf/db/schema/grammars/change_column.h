/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id change_column.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/03/13 10:21
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_GRAMMARS_CHAGE_COLUMN_H_
#define PF_DB_SCHEMA_GRAMMARS_CHAGE_COLUMN_H_

#include "pf/db/schema/grammars/config.h"

namespace pf_db {

namespace schema {

namespace grammars {

class PF_API ChangeColumn {

 public:
   ChangeColumn() {}
   ~ChangeColumn() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;

 public:

   //Compile a change column command into a series of SQL statements.
   static variable_array_t compile(Grammar *grammar, 
                                   Blueprint *blueprint, 
                                   const std::string &command, 
                                   Connection *connection);

 protected:

   //Get the Doctrine table difference for the given changes.
   static bool get_changed_diff(
       Grammar *grammar, Blueprint *blueprint, void *schema);

   //Get a copy of the given Doctrine table after making the column changes.
   static bool get_table_with_column_changes(Blueprint *blueprint, void *table);

   //Get the Doctrine column instance for a column change.
   static bool get_doctrine_column(void *table, const std::string &fluent);

   //Get the Doctrine column change options.
   static bool get_doctrine_column_change_options(const std::string &fluent);

   //Get the doctrine column type.
   static int32_t get_doctrine_column_type(const std::string &type);

   //Calculate the proper column length to force the Doctrine text type.
   static int32_t calculate_doctrine_text_length(const std::string &type);

   //Get the matching Doctrine option for a given Fluent attribute name.
   static std::string map_fluent_option_to_doctrine(const std::string &attribute);

   //Get the matching Doctrine value for a given Fluent attribute.
   static variable_t map_fluent_value_to_doctrine(
       const std::string &option, const variable_t &value);

};

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_CHAGE_COLUMN_H_
