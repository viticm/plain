/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id rename_column.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/04/13 15:10
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_GRAMMARS_RENAME_COLUMN_H_
#define PF_DB_SCHEMA_GRAMMARS_RENAME_COLUMN_H_

#include "pf/db/schema/grammars/config.h"
#include "pf/support/fluent.h"

namespace pf_db {

namespace schema {

namespace grammars {

class PF_API RenameColumn {

 public:
   RenameColumn() {}
   ~RenameColumn() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using Fluent = pf_support::Fluent;

 public:

   //Compile a change column command into a series of SQL statements.
   static variable_array_t compile(Grammar *grammar, 
                                   Blueprint *blueprint, 
                                   Fluent &command, 
                                   Connection *connection);
}

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_RENAME_COLUMN_H_
