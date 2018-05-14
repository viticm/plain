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

   SqliteGrammar();
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
   }

   //Compile the query to determine the list of columns.
   virtual std::string compile_column_listing(const std::string &table);

   //Compile a create table command.
   virtual std::string compile_create(Blueprint *blueprint, 
                                      fluent_t &command, 
                                      ConnectionInterface *connection);

   //Compile an add column command.
   virtual std::string compile_add(Blueprint *blueprint, fluent_t &command);

   //Compile a primary key command.
   virtual std::string compile_primary(Blueprint *, fluent_t &) {
     return "";
   };

   //Compile a unique key command.
   virtual std::string compile_unique(Blueprint *blueprint, fluent_t &command);

   //Compile a plain index key command.
   virtual std::string compile_index(Blueprint *blueprint, fluent_t &command);

   //Compile a foreign key command.
   virtual std::string compile_foreign(Blueprint *, fluent_t &) {
     return "";
   }

   //Compile a drop table command.
   virtual std::string compile_drop(Blueprint *blueprint, fluent_t &) {
     return "drop table " + wrap_table(blueprint);
   }

   //Compile a drop table (if exists) command.
   virtual std::string compile_drop_if_exists(
       Blueprint *blueprint, fluent_t &) {
     return "drop table if exists " + wrap_table(blueprint);
   }

   //Compile a drop column command.
   virtual std::string compile_drop_column(
       Blueprint *blueprint, fluent_t &command);

   //Compile a drop primary key command.
   virtual std::string compile_drop_primary(
       Blueprint *, fluent_t &) {
     return "";
   }

   //Compile a drop unique key command.
   virtual std::string compile_drop_unique(
       Blueprint *, fluent_t &command) {
     auto index = wrap(command["index"]);
     return "drop index " + index;
   }

   //Compile a drop index command.
   virtual std::string compile_drop_index(
       Blueprint *, fluent_t &command) {
     auto index = wrap(command["index"]);
     return "drop index " + index;
   }

   //Compile a drop foreign key command.
   virtual std::string compile_drop_foreign(Blueprint *, fluent_t &) {
     return "";
   }

   //Compile a rename table command.
   virtual std::string compile_rename(Blueprint *blueprint, fluent_t &command) {
     auto from = wrap_table(blueprint);
     return "alter table " + from + " rename to " + wrap_table(command["to"]);
   }

   //Compile the command to enable foreign key constraints.
   virtual std::string compile_enable_foreign_key_constraints() const {
     return "PRAGMA foreign_keys = ON;";
   }

   //Compile the command to disable foreign key constraints.
   virtual std::string compile_disable_foreign_key_constraints() const {
     return "PRAGMA foreign_keys = OFF;";
   }

 protected:

   //Create the column definition for a char type.
   virtual std::string type_char(fluent_t &) const {
     return "varchar";
   }

   //Create the column definition for a string type.
   virtual std::string type_string(fluent_t &) const {
     return "varchar";
   }

   //Create the column definition for a text type.
   virtual std::string type_text(fluent_t &) const {
     return "text";
   }

   //Create the column definition for a medium text type.
   virtual std::string type_medium_text(fluent_t &) const {
     return "text";
   }

   //Create the column definition for a long text type.
   virtual std::string type_long_text(fluent_t &) const {
     return "text";
   }

   //Create the column definition for a big integer type.
   virtual std::string type_big_integer(fluent_t &) const {
     return "integer";
   }

   //Create the column definition for an integer type.
   virtual std::string type_integer(fluent_t &) const {
     return "integer";
   }

   //Create the column definition for a medium integer type.
   virtual std::string type_medium_integer(fluent_t &) const {
     return "integer";
   }

   //Create the column definition for a tiny integer type.
   virtual std::string type_tiny_integer(fluent_t &) const {
     return "integer";
   }

   //Create the column definition for a small integer type.
   virtual std::string type_small_integer(fluent_t &) const {
     return "integer";
   }

   //Create the column definition for a float type.
   virtual std::string type_float(fluent_t &) const {
     return "float";
   }

   //Create the column definition for a double type.
   virtual std::string type_double(fluent_t &) const {
     return "float";
   }

   //Create the column definition for a decimal type.
   virtual std::string type_decimal(fluent_t &) const {
     return "numeric";
   } 

   //Create the column definition for a boolean type.
   virtual std::string type_boolean(fluent_t &) const {
     return "tinyint(1)";
   }

   //Create the column definition for an enum type.
   virtual std::string type_enum(fluent_t &) const {
     return "varchar";
   }

   //Create the column definition for a json type.
   virtual std::string type_json(fluent_t &) const {
     return "text";
   }

   //Create the column definition for a jsonb type.
   virtual std::string type_jsonb(fluent_t &) const {
     return "text";
   } 

   //Create the column definition for a date type.
   virtual std::string type_date(fluent_t &) const {
     return "date";
   } 

   //Create the column definition for a date-time type.
   virtual std::string type_date_time(fluent_t &) const {
     return "datetime";
   }

   //Create the column definition for a date-time type.
   //Note: "SQLite does not have a storage class set aside for storing dates and/or times." 
   //@link https://www.sqlite.org/datatype3.html
   virtual std::string type_date_time_tz(fluent_t &) const {
     return "datetime";
   }

   //Create the column definition for a time type.
   virtual std::string type_time(fluent_t &) const {
     return "time";
   }

   //Create the column definition for a time type.
   virtual std::string type_time_tz(fluent_t &) const {
     return "time";
   }

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp(fluent_t &column) const {
     if (column["use_current"] == true) 
       return "datetime default CURRENT_TIMESTAMP";
     return "datetime";
   }

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp_tz(fluent_t &column) const {
      if (column["use_current"] == true) 
       return "datetime default CURRENT_TIMESTAMP";
     return "datetime";
   }

   //Create the column definition for a binary type.
   virtual std::string type_binary(fluent_t &) const {
     return "blob";
   }

   //Create the column definition for a uuid type.
   virtual std::string type_uuid(fluent_t &) const {
     return "varchar";
   }

   //Create the column definition for an IP address type.
   virtual std::string type_ip_address(fluent_t &) const {
     return "varchar";
   }

   //Create the column definition for a MAC address type.
   virtual std::string type_mac_address(fluent_t &) const {
     return "varchar";
   }

 protected:

   //Get the SQL for an nullable column modifier.
   virtual std::string modify_nullable(Blueprint *, fluent_t &column) {
     return column["nullable"] == true ? " null" : " not null";
   }

   //Get the SQL for a generated virtual column modifier.
   virtual std::string modify_virtual_as(Blueprint *, fluent_t &) {
     return "";
   }

   //Get the SQL for a generated stored column modifier.
   virtual std::string modify_stored_as(Blueprint *, fluent_t &) {
     return "";
   }

   //Get the SQL for an unsigned column modifier.
   virtual std::string modify_unsigned(Blueprint *, fluent_t &) {
     return "";
   }

   //Get the SQL for a character set column modifier.
   virtual std::string modify_charset(Blueprint *, fluent_t &) {
     return "";
   }

   //Get the SQL for a collation column modifier.
   virtual std::string modify_collate(Blueprint *, fluent_t &) {
     return "";
   }

   //Get the SQL for a default column modifier.
   virtual std::string modify_default(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for an auto-increment column modifier.
   virtual std::string modify_increment(Blueprint *blueprint, fluent_t &column);

 protected:

   //Get the foreign key syntax for a table creation statement.
   std::string add_foreign_keys(Blueprint *blueprint);

   //Get the SQL for the foreign key.
   std::string get_foreign_key(fluent_t &foreign);

   //Get the primary key syntax for a table creation statement.
   std::string add_primary_keys(Blueprint *blueprint);

};

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_SQLITE_GRAMMAR_H_
