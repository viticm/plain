/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id postgres_grammar.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/04/13 14:20
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_GRAMMARS_POSTGRES_GRAMMAR_H_
#define PF_DB_SCHEMA_GRAMMARS_POSTGRES_GRAMMAR_H_

#include "pf/db/schema/grammars/grammar.h"

namespace pf_db {

namespace schema {

namespace grammars {

class PF_API PostgresGrammar : public Grammar {

 public:
   PostgresGrammar();
   ~PostgresGrammar() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using fluent_t = db_schema_fluent_t;

 public:

   //Compile the query to determine the list of tables.
   virtual std::string compile_table_exists() const {
     return "select * from information_schema.tables where table_schema = ? \
and table_name = ?";
   }

   //Compile the query to determine the list of columns.
   virtual std::string compile_column_listing(const std::string &table) {
     return "select column_name from information_schema.columns where \
table_name = '" + table + "'";
   }

   //Compile a create table command.
   virtual std::string compile_create(Blueprint *blueprint, 
                                      fluent_t &command, 
                                      ConnectionInterface *connection);

   //Compile an add column command.
   virtual std::string compile_add(Blueprint *blueprint, fluent_t &command);

   //Compile a primary key command.
   virtual std::string compile_primary(Blueprint *blueprint, fluent_t &command) {
     auto columns = columnize(command.columns);
     return "alter table " + 
            wrap_table(blueprint) + " add primary key (" + columns + ")";
   }

   //Compile a unique key command.
   virtual std::string compile_unique(Blueprint *blueprint, fluent_t &command);

   //Compile a plain index key command.
   virtual std::string compile_index(Blueprint *blueprint, fluent_t &command);

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
   virtual std::string compile_drop_primary(Blueprint *blueprint, fluent_t &);

   //Compile a drop unique key command.
   virtual std::string compile_drop_unique(
       Blueprint *blueprint, fluent_t &command) {
     std::string index = wrap(command["index"]);
     return "alter table " + wrap_table(blueprint) + " drop constraint " + index;
   }

   //Compile a drop index command.
   virtual std::string compile_drop_index(Blueprint *, fluent_t &command) {
     return "drop index " + wrap(command["index"]);
   }

   //Compile a drop foreign key command.
   virtual std::string compile_drop_foreign(
       Blueprint *blueprint, fluent_t &command) {
     std::string index = wrap(command["index"]);
     return "alter table " + wrap_table(blueprint) + " drop constraint " + index;
   }

   //Compile a rename table command.
   virtual std::string compile_rename(Blueprint *blueprint, fluent_t &command) {
     auto from = wrap_table(blueprint);
     return "alter table " + from + " rename to " + wrap_table(command["to"]);
   }

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
   virtual std::string type_char(fluent_t &column) const {
     return "char(" + column["length"].data + ")";
   }

   //Create the column definition for a string type.
   virtual std::string type_string(fluent_t &column) const {
     return "varchar(" + column["length"].data + ")";
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
   virtual std::string type_big_integer(fluent_t &column) const {
     return column["auto_increment"] == true ? "bigserial" : "bigint";
   }

   //Create the column definition for an integer type.
   virtual std::string type_integer(fluent_t &column) const {
     return column["auto_increment"] == true ? "serial" : "integer";
   }

   //Create the column definition for a medium integer type.
   virtual std::string type_medium_integer(fluent_t &column) const {
     return column["auto_increment"] == true ? "serial" : "integer";
   }

   //Create the column definition for a tiny integer type.
   virtual std::string type_tiny_integer(fluent_t &column) const {
     return column["auto_increment"] == true ? "smallserial" : "smallint";
   }

   //Create the column definition for a small integer type.
   virtual std::string type_small_integer(fluent_t &column) const {
     return column["auto_increment"] == true ? "smallserial" : "smallint";
   }

   //Create the column definition for a float type.
   virtual std::string type_float(fluent_t &column) const {
     return type_double(column); 
   }

   //Create the column definition for a double type.
   virtual std::string type_double(fluent_t &) const {
     return "double precision";
   }

   //Create the column definition for a real type.
   virtual std::string type_real(fluent_t &) const {
     return "real";
   }

   //Create the column definition for a decimal type.
   virtual std::string type_decimal(fluent_t &column) const {
     return "decimal(" + 
            column["total"].data + ", " + column["places"].data + ")";
   } 

   //Create the column definition for a boolean type.
   virtual std::string type_boolean(fluent_t &) const {
     return "boolean";
   }

   //Create the column definition for an enum type.
   virtual std::string type_enum(fluent_t &column) const;

   //Create the column definition for a json type.
   virtual std::string type_json(fluent_t &) const {
     return "json";
   }

   //Create the column definition for a jsonb type.
   virtual std::string type_jsonb(fluent_t &) const {
     return "jsonb";
   } 

   //Create the column definition for a date type.
   virtual std::string type_date(fluent_t &) const {
     return "date";
   } 

   //Create the column definition for a date-time type.
   virtual std::string type_date_time(fluent_t &) const {
     return "timestamp(0) without time zone";
   }

   //Create the column definition for a date-time type.
   virtual std::string type_date_time_tz(fluent_t &) const {
     return "timestamp(0) with time zone";
   }

   //Create the column definition for a time type.
   virtual std::string type_time(fluent_t &) const {
     return "time(0) without time zone";
   }

   //Create the column definition for a time type.
   virtual std::string type_time_tz(fluent_t &) const {
     return "time(0) with time zone";
   }

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp(fluent_t &column) const {
     if (column["use_current"] == true)
       return "timestamp(0) without time zone default CURRENT_TIMESTAMP(0)";
     return "timestamp(0) without time zone";
   }

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp_tz(fluent_t &column) const {
     if (column["use_current"] == true)
       return "timestamp(0) with time zone default CURRENT_TIMESTAMP(0)";
     return "timestamp(0) with time zone";
   }


   //Create the column definition for a binary type.
   virtual std::string type_binary(fluent_t &) const {
     return "bytea";
   }

   //Create the column definition for a uuid type.
   virtual std::string type_uuid(fluent_t &) const {
     return "uuid";
   }

   //Create the column definition for an IP address type.
   virtual std::string type_ip_address(fluent_t &) const {
     return "inet";
   }

   //Create the column definition for a MAC address type.
   virtual std::string type_mac_address(fluent_t &) const {
     return "macaddr";
   }

 protected:

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

   //Get the SQL for a nullable column modifier.
   virtual std::string modify_nullable(Blueprint *, fluent_t &column) {
     return column["nullable"] == true ? " null" : " not null";
   }

   //Get the SQL for a default column modifier.
   virtual std::string modify_default(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for an auto-increment column modifier.
   virtual std::string modify_increment(Blueprint *, fluent_t &column);

};

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_POSTGRES_GRAMMAR_H_
