/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id grammar.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/03/13 10:52
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_GRAMMARS_GRAMMAR_H_
#define PF_DB_SCHEMA_GRAMMARS_GRAMMAR_H_

#include "pf/db/schema/grammars/config.h"
#include "pf/db/grammar.h"

namespace pf_db {

namespace schema {

namespace grammars {

class PF_API Grammar : public pf_db::Grammar {

 public:
   Grammar();
   ~Grammar() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using fluent_t = db_schema_fluent_t;

 public:

   //Compile a rename column command.
   virtual std::vector<std::string> compile_rename_column(
       Blueprint *blueprint, fluent_t &command, ConnectionInterface *connection);

   //Compile a change column command into a series of SQL statements.
   virtual std::vector<std::string> compile_change(
       Blueprint *blueprint, fluent_t &command, ConnectionInterface *connection);

   //Compile a foreign key command.
   virtual std::string compile_foreign(Blueprint *blueprint, fluent_t &command);

   //Add a prefix to an array of values.
   variable_array_t prefix_array(
       const std::string &prefix, const variable_array_t &values);

   //Wrap a table in keyword identifiers.
   virtual std::string wrap_table(const variable_t &table);

   //Wrap a value in keyword identifiers.
   virtual std::string wrap(const variable_t &value, bool prefix_alias = false);

   //Create an empty Doctrine DBAL TableDiff from the Blueprint.
   bool get_doctrine_table_diff(Blueprint *blueprint, void *schema);

   //Compile the query to determine the list of tables.
   virtual std::string compile_table_exists() const = 0;

   //Compile the query to determine the list of columns.
   virtual std::string compile_column_listing(const std::string &table) const = 0;

   //Compile the query to determine the list of columns.
   virtual std::string compile_column_listing() const = 0;

   //Compile the command to enable foreign key constraints.
   virtual std::string compile_enable_foreign_key_constraints() const = 0;

   //Compile the command to disable foreign key constraints.
   virtual std::string compile_disable_foreign_key_constraints() const = 0;

   //Compile a create table command.
   virtual std::string compile_create(Blueprint *blueprint, 
                                      fluent_t &command, 
                                      ConnectionInterface *connection) = 0;

   //Compile an add column command.
   virtual std::string compile_add(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a primary key command.
   virtual std::string compile_primary(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a unique key command.
   virtual std::string compile_unique(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a plain index key command.
   virtual std::string compile_index(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop table command.
   virtual std::string compile_drop(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop table (if exists) command.
   virtual std::string compile_drop_if_exists(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop column command.
   virtual std::string compile_drop_column(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop primary key command.
   virtual std::string compile_drop_primary(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop unique key command.
   virtual std::string compile_drop_unique(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop index command.
   virtual std::string compile_drop_index(
       Blueprint *blueprint, fluent_t &command) = 0;

   //Compile a drop foreign key command.
   virtual std::string compile_drop_foreign(
       Blueprint *blueprint, fluent_t &command) = 0;

   //The function call compile.
   std::vector<std::string> call_compile(
       Blueprint *blueprint, 
       fluent_t &command, 
       ConnectionInterface *connection, 
       const std::string &method);

 protected:

   //If this Grammar supports schema changes wrapped in a transaction.
   bool transactions_;

   //The compile hash functions.
   std::map< std::string, 
     std::function<std::string(Blueprint *, fluent_t &)> > compile_calls_;

 protected:

   //Compile the blueprint's column definitions.
   std::vector<std::string> get_columns(Blueprint *blueprint);

   //Get the SQL for the column data type.
   std::string get_type(const std::string &column);

   //Add the column modifiers to the definition.
   std::string add_modifiers(const std::string &sql, 
                             Blueprint *blueprint, 
                             const std::string &column);

   //Get the primary key command if it exists on the blueprint.
   std::string get_command_by_name(Blueprint *blueprint, const std::string &name);

   //Get all of the commands with a given name.
   std::vector<std::string> get_commands_by_name(
       Blueprint *blueprint, const std::string &name);

   //Format a value so that it can be used in "default" clauses.
   std::string get_default_value(const variable_t &value);

   //Check if this Grammar supports schema changes wrapped in a transaction.
   bool supports_schema_transactions() const {
     return transactions_;
   }

};

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_GRAMMAR_H_
