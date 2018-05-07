/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id sqlite_grammar.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/04/13 14:50
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_GRAMMARS_SQLITE_GRAMMAR_H_
#define PF_DB_SCHEMA_GRAMMARS_SQLITE_GRAMMAR_H_

#include "pf/db/schema/grammars/grammar.h"

namespace pf_db {

namespace schema {

namespace grammars {

class PF_API SqliteGrammar : public Grammar {

 public:

   SqliteGrammar() {}
   ~SqliteGrammar() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using fluent_t = db_schema_fluent_t;

 public:

   //Compile the query to determine the list of tables.
   virtual std::string compile_table_exists() const {
     return "select * from sqlite_master where type = 'table' and name = ?";
   };

   //Compile the query to determine the list of columns.
   virtual std::string compile_column_listing(const std::string &table) const {
     return "pragma table_info(" + 
       wrap_table(str_replace(".", "__", table)) + ")";
   }

   //Compile a create table command.
   virtual std::string compile_create(Blueprint *blueprint, 
                                      fluent_t &command, 
                                      Connection *connection);

   //Compile an add column command.
   virtual std::string compile_add(Blueprint *blueprint, fluent_t &command);

   //Compile a primary key command.
   virtual std::string compile_primary(Blueprint *blueprint, fluent_t &command);

   //Compile a unique key command.
   virtual std::string compile_unique(Blueprint *blueprint, fluent_t &command);

   //Compile a plain index key command.
   virtual std::string compile_index(Blueprint *blueprint, fluent_t &command);

   //Compile a drop table command.
   virtual std::string compile_drop(Blueprint *blueprint, fluent_t &command);

   //Compile a drop table (if exists) command.
   virtual std::string compile_drop_if_exists(
       Blueprint *blueprint, fluent_t &command);

   //Compile a drop column command.
   virtual std::string compile_drop_column(
       Blueprint *blueprint, fluent_t &command);

   //Compile a drop primary key command.
   virtual std::string compile_drop_primary(
       Blueprint *blueprint, fluent_t &command);

   //Compile a drop unique key command.
   virtual std::string compile_drop_unique(
       Blueprint *blueprint, fluent_t &command);

   //Compile a drop index command.
   virtual std::string compile_drop_index(
       Blueprint *blueprint, fluent_t &command);

   //Compile a drop foreign key command.
   virtual std::string compile_drop_foreign(
       Blueprint *blueprint, fluent_t &command);

   //Compile a rename table command.
   virtual std::string compile_rename(Blueprint *blueprint, fluent_t &command);

   //Compile the command to enable foreign key constraints.
   virtual std::string compile_enable_foreign_key_constraints() const {
     return "SET CONSTRAINTS ALL IMMEDIATE;";
   }

   //Compile the command to disable foreign key constraints.
   virtual std::string compile_disable_foreign_key_constraints() const {
     return "SET CONSTRAINTS ALL DEFERRED;";
   }

 protected:

   //Create the column definition for a char type.
   std::string type_char(fluent_t &column) const;

   //Create the column definition for a string type.
   std::string type_string(fluent_t &column) const;

   //Create the column definition for a text type.
   std::string type_text(fluent_t &column) const;

   //Create the column definition for a medium text type.
   std::string type_medium_text(fluent_t &column) const;

   //Create the column definition for a long text type.
   std::string type_long_text(fluent_t &column) const;

   //Create the column definition for a big integer type.
   std::string type_big_integer(fluent_t &column) const;

   //Create the column definition for an integer type.
   std::string type_integer(fluent_t &column) const;

   //Create the column definition for a medium integer type.
   std::string type_medium_integer(fluent_t &column) const;

   //Create the column definition for a tiny integer type.
   std::string type_tiny_integer(fluent_t &column) const;

   //Create the column definition for a small integer type.
   std::string type_small_integer(fluent_t &column) const;

   //Create the column definition for a float type.
   std::string type_float(fluent_t &column) const;

   //Create the column definition for a double type.
   std::string type_double(fluent_t &column) const;

   //Create the column definition for a decimal type.
   std::string type_decimal(fluent_t &column) const; 

   //Create the column definition for a boolean type.
   std::string type_boolean(fluent_t &column) const;

   //Create the column definition for an enum type.
   std::string type_enum(fluent_t &column) const;

   //Create the column definition for a json type.
   std::string type_json(fluent_t &column) const;

   //Create the column definition for a jsonb type.
   std::string type_jsonb(fluent_t &column) const; 

   //Create the column definition for a date type.
   std::string type_date(fluent_t &column) const; 

   //Create the column definition for a date-time type.
   std::string type_date_time(fluent_t &column) const;

   //Create the column definition for a date-time type.
   std::string type_date_time_tz(fluent_t &column) const;

   //Create the column definition for a time type.
   std::string type_time(fluent_t &column) const;

   //Create the column definition for a time type.
   std::string type_time_tz(fluent_t &column) const;

   //Create the column definition for a timestamp type.
   std::string type_timestamp(fluent_t &column) const;

   //Create the column definition for a binary type.
   std::string type_blob(fluent_t &column) const;

   //Create the column definition for a uuid type.
   std::string type_uuid(fluent_t &column) const;

   //Create the column definition for an IP address type.
   std::string type_ip_address(fluent_t &column) const;

   //Create the column definition for a MAC address type.
   std::string type_mac_address(fluent_t &column) const;

   //Get the SQL for a generated virtual column modifier.
   std::string modify_virtual_as(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for a generated stored column modifier.
   std::string modify_stored_as(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for an unsigned column modifier.
   std::string modify_unsigned(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for a character set column modifier.
   std::string modify_character(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for a collation column modifier.
   std::string modify_collation(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for a default column modifier.
   std::string modify_default(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for an auto-increment column modifier.
   std::string modify_increment(Blueprint *blueprint, fluent_t &column);

   //Wrap a single string in keyword identifiers.
   virtual std::string wrap_value(const variable_t &value);

}

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_SQLITE_GRAMMAR_H_
