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
   virtual std::string wrap_table(Blueprint *blueprint);

   //Wrap a table in keyword identifiers.
   virtual std::string wrap_table(const variable_t &value);

   //Wrap a value in keyword identifiers.
   virtual std::string wrap(const variable_t &value, bool prefix_alias = false);

   //Wrap a value in keyword identifiers.
   virtual std::string wrap(fluent_t &value) {
     return wrap(value["name"]);
   };

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

   //The function call type.
   std::string call_type(fluent_t &column, const std::string &method);

   //The function call modify.
   std::string call_modify(
       Blueprint *blueprint, fluent_t &column, const std::string &method);

 protected:

   //If this Grammar supports schema changes wrapped in a transaction.
   bool transactions_;

   //The compile hash functions.
   std::map< std::string, 
     std::function<std::string(Blueprint *, fluent_t &)> > compile_calls_;

   //The type hash functions.
   std::map< std::string, 
     std::function<std::string(fluent_t &)> > type_calls_;

   //The type hash functions.
   std::map< std::string, 
     std::function<std::string(Blueprint *, fluent_t &)> > modify_calls_;

 protected:

   //The possible column modifiers.
   std::vector<std::string> modifiers_;

   //The possible column serials.
   std::vector<std::string> serials_;

 protected:

   //Compile the blueprint's column definitions.
   std::vector<std::string> get_columns(Blueprint *blueprint);

   //Get the SQL for the column data type.
   std::string get_type(fluent_t &column);

   //Add the column modifiers to the definition.
   std::string add_modifiers(const std::string &sql, 
                             Blueprint *blueprint, 
                             fluent_t &column);

   //Get the primary key command if it exists on the blueprint.
   fluent_t get_command_by_name(Blueprint *blueprint, const std::string &name);

   //Get all of the commands with a given name.
   std::vector<fluent_t> get_commands_by_name(
       Blueprint *blueprint, const std::string &name);

   //Format a value so that it can be used in "default" clauses.
   std::string get_default_value(const variable_t &value);

   //Check if this Grammar supports schema changes wrapped in a transaction.
   bool supports_schema_transactions() const {
     return transactions_;
   }

 protected:

   //Create the column definition for a char type.
   virtual std::string type_char(fluent_t &column) const = 0;

   //Create the column definition for a string type.
   virtual std::string type_string(fluent_t &column) const = 0;

   //Create the column definition for a text type.
   virtual std::string type_text(fluent_t &column) const = 0;

   //Create the column definition for a medium text type.
   virtual std::string type_medium_text(fluent_t &column) const = 0;

   //Create the column definition for a long text type.
   virtual std::string type_long_text(fluent_t &column) const = 0;

   //Create the column definition for a big integer type.
   virtual std::string type_big_integer(fluent_t &column) const = 0;

   //Create the column definition for an integer type.
   virtual std::string type_integer(fluent_t &column) const = 0;

   //Create the column definition for a medium integer type.
   virtual std::string type_medium_integer(fluent_t &column) const = 0;

   //Create the column definition for a tiny integer type.
   virtual std::string type_tiny_integer(fluent_t &column) const = 0;

   //Create the column definition for a small integer type.
   virtual std::string type_small_integer(fluent_t &column) const = 0;

   //Create the column definition for a float type.
   virtual std::string type_float(fluent_t &column) const = 0;

   //Create the column definition for a double type.
   virtual std::string type_double(fluent_t &column) const = 0;

   //Create the column definition for a decimal type.
   virtual std::string type_decimal(fluent_t &column) const = 0; 

   //Create the column definition for a boolean type.
   virtual std::string type_boolean(fluent_t &column) const = 0;

   //Create the column definition for an enum type.
   virtual std::string type_enum(fluent_t &column) const = 0;

   //Create the column definition for a json type.
   virtual std::string type_json(fluent_t &column) const = 0;

   //Create the column definition for a jsonb type.
   virtual std::string type_jsonb(fluent_t &column) const = 0; 

   //Create the column definition for a date type.
   virtual std::string type_date(fluent_t &column) const = 0; 

   //Create the column definition for a date-time type.
   virtual std::string type_date_time(fluent_t &column) const = 0;

   //Create the column definition for a date-time type.
   virtual std::string type_date_time_tz(fluent_t &column) const = 0;

   //Create the column definition for a time type.
   virtual std::string type_time(fluent_t &column) const = 0;

   //Create the column definition for a time type.
   virtual std::string type_time_tz(fluent_t &column) const = 0;

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp(fluent_t &column) const = 0;

   //Create the column definition for a binary type.
   virtual std::string type_blob(fluent_t &column) const = 0;

   //Create the column definition for a uuid type.
   virtual std::string type_uuid(fluent_t &column) const = 0;

   //Create the column definition for an IP address type.
   virtual std::string type_ip_address(fluent_t &column) const = 0;

   //Create the column definition for a MAC address type.
   virtual std::string type_mac_address(fluent_t &column) const = 0;

 protected:

   //Get the SQL for a generated virtual column modifier.
   virtual std::string modify_virtual_as(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for a generated stored column modifier.
   virtual std::string modify_stored_as(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for an unsigned column modifier.
   virtual std::string modify_unsigned(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for a character set column modifier.
   virtual std::string modify_character(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for a collation column modifier.
   virtual std::string modify_collation(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for a default column modifier.
   virtual std::string modify_default(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for an auto-increment column modifier.
   virtual std::string modify_increment(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for a "first" column modifier.
   virtual std::string modify_first(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for an "after" column modifier.
   virtual std::string modify_after(Blueprint *blueprint, fluent_t &column) = 0;

   //Get the SQL for a "comment" column modifier.
   virtual std::string modify_comment(Blueprint *blueprint, fluent_t &column) = 0;

};

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_GRAMMAR_H_
