/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id sqlserver_grammar.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/04/13 15:04
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_GRAMMARS_SQLSERVER_GRAMMAR_H_
#define PF_DB_SCHEMA_GRAMMARS_SQLSERVER_GRAMMAR_H_

#include "pf/db/schema/grammars/grammar.h"

namespace pf_db {

namespace schema {

namespace grammars {

class PF_API SqlserverGrammar : public Grammar {

 public:

   SqlserverGrammar();
   ~SqlserverGrammar() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using fluent_t = db_schema_fluent_t;

 public:

   //Compile the query to determine the list of tables.
   virtual std::string compile_table_exists() const {
     return "select * from sysobjects where type = 'U' and name = ?";
   };

   //Compile the query to determine the list of columns.
   virtual std::string compile_column_listing(const std::string &table) {
     return "select col.name from sys.columns as col \
join sys.objects as obj on col.object_id = obj.object_id \
where obj.type = 'U' and obj.name = '" + table + "'";
   }

   //Compile a create table command.
   virtual std::string compile_create(Blueprint *blueprint, 
                                      fluent_t &command, 
                                      ConnectionInterface *connection);

   //Compile an add column command.
   virtual std::string compile_add(Blueprint *blueprint, fluent_t &command);

   //Compile a primary key command.
   virtual std::string compile_primary(Blueprint *blueprint, fluent_t &command);

   //Compile a unique key command.
   virtual std::string compile_unique(Blueprint *blueprint, fluent_t &command);

   //Compile a plain index key command.
   virtual std::string compile_index(Blueprint *blueprint, fluent_t &command);

   //Compile a drop table command.
   virtual std::string compile_drop(Blueprint *blueprint, fluent_t &command) {
     return "drop table " + wrap_table(blueprint);
   };

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
       Blueprint *blueprint, fluent_t &command) {
     auto index = wrap(command["index"]);
     return "drop index " + index + " on " + wrap_table(blueprint);
   };

   //Compile a drop foreign key command.
   virtual std::string compile_drop_foreign(
       Blueprint *blueprint, fluent_t &command) {
     auto index = wrap(command["index"]);
     return "alter table " + wrap_table(blueprint) + " drop constraint " + index;
   };

   //Compile a rename table command.
   virtual std::string compile_rename(Blueprint *blueprint, fluent_t &command) {
     auto from = wrap_table(blueprint);
     return "sp_rename " + from + ", " + wrap_table(command["to"]);
   };

   //Compile the command to enable foreign key constraints.
   virtual std::string compile_enable_foreign_key_constraints() const {
     return "EXEC sp_msforeachtable @command1=\"print \'?\'\", "
       "@command2=\"ALTER TABLE ? WITH CHECK CHECK CONSTRAINT all\";";
   }

   //Compile the command to disable foreign key constraints.
   virtual std::string compile_disable_foreign_key_constraints() const {
     return "EXEC sp_msforeachtable \"ALTER TABLE ? NOCHECK CONSTRAINT all\";";
   }

 protected:

   //Create the column definition for a char type.
   virtual std::string type_char(fluent_t &column) const {
     return "nchar(" + column["length"].data + ")";
   };

   //Create the column definition for a string type.
   virtual std::string type_string(fluent_t &column) const {
     return "nvarchar(" + column["length"].data + ")"; 
   };

   //Create the column definition for a text type.
   virtual std::string type_text(fluent_t &) const {
     return "nvarchar(max)";
   };

   //Create the column definition for a medium text type.
   virtual std::string type_medium_text(fluent_t &) const {
     return "nvarchar(max)";
   };

   //Create the column definition for a long text type.
   virtual std::string type_long_text(fluent_t &) const {
     return "nvarchar(max)";
   };

   //Create the column definition for a big integer type.
   virtual std::string type_big_integer(fluent_t &) const {
     return "bigint";
   };

   //Create the column definition for an integer type.
   virtual std::string type_integer(fluent_t &) const {
     return "int";
   };

   //Create the column definition for a medium integer type.
   virtual std::string type_medium_integer(fluent_t &) const {
     return "int";
   };

   //Create the column definition for a tiny integer type.
   virtual std::string type_tiny_integer(fluent_t &) const {
     return "tinyint";
   };

   //Create the column definition for a small integer type.
   virtual std::string type_small_integer(fluent_t &) const {
     return "smallint";
   };

   //Create the column definition for a float type.
   virtual std::string type_float(fluent_t &) const {
     return "float";
   };

   //Create the column definition for a double type.
   virtual std::string type_double(fluent_t &) const {
     return "float";
   };

   //Create the column definition for a decimal type.
   virtual std::string type_decimal(fluent_t &column) const {
     return "decimal(" + column["total"].data + ", " + column["places"].data + ")";
   }; 

   //Create the column definition for a boolean type.
   virtual std::string type_boolean(fluent_t &) const {
     return "bit";
   };

   //Create the column definition for an enum type.
   virtual std::string type_enum(fluent_t &) const {
     return "nvarchar(255)";
   };

   //Create the column definition for a json type.
   virtual std::string type_json(fluent_t &) const {
     return "nvarchar(max)";
   };

   //Create the column definition for a jsonb type.
   virtual std::string type_jsonb(fluent_t &) const {
     return "nvarchar(max)";
   }; 

   //Create the column definition for a date type.
   virtual std::string type_date(fluent_t &) const {
     return "date";
   }; 

   //Create the column definition for a date-time type.
   virtual std::string type_date_time(fluent_t &) const {
     return "datetime";
   };

   //Create the column definition for a date-time type.
   virtual std::string type_date_time_tz(fluent_t &) const {
     return "datetimeoffset(0)";
   };

   //Create the column definition for a time type.
   virtual std::string type_time(fluent_t &) const {
     return "time";
   };

   //Create the column definition for a time type.
   virtual std::string type_time_tz(fluent_t &) const {
     return "time";
   };

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp(fluent_t &column) const {
     if (column["use_current"])
       return "datetime default CURRENT_TIMESTAMP";
     return "datetime";
   };

   //Create the column definition for a timestamp type.
   virtual std::string type_timestamp_tz(fluent_t &column) const {
     if (column["use_current"]) 
       return "datetimeoffset(0) default CURRENT_TIMESTAMP";
     return "datetimeoffset(0)";
   };

   //Create the column definition for a binary type.
   virtual std::string type_binary(fluent_t &) const {
     return "varbinary(max)";
   };

   //Create the column definition for a uuid type.
   virtual std::string type_uuid(fluent_t &) const {
     return "uniqueidentifier";
   };

   //Create the column definition for an IP address type.
   virtual std::string type_ip_address(fluent_t &) const {
     return "nvarchar(45)";
   };

   //Create the column definition for a MAC address type.
   virtual std::string type_mac_address(fluent_t &) const {
     return "nvarchar(17)";
   };

 protected:

   //Get the SQL for a generated virtual column modifier.
   virtual std::string modify_virtual_as(Blueprint *, fluent_t &) {
     return "";
   };

   //Get the SQL for a generated stored column modifier.
   virtual std::string modify_stored_as(Blueprint *, fluent_t &) {
     return "";
   };

   //Get the SQL for an unsigned column modifier.
   virtual std::string modify_unsigned(Blueprint *, fluent_t &) {
     return "";
   };

   //Get the SQL for a character set column modifier.
   virtual std::string modify_charset(Blueprint *, fluent_t &) {
     return "";
   };

   //Get the SQL for a collation column modifier.
   virtual std::string modify_collate(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for an nullable column modifier.
   virtual std::string modify_nullable(Blueprint *, fluent_t &column) {
     return column["column"] == true ? " null" : " not null";
   };

   //Get the SQL for a default column modifier.
   virtual std::string modify_default(Blueprint *blueprint, fluent_t &column);

   //Get the SQL for an auto-increment column modifier.
   virtual std::string modify_increment(Blueprint *blueprint, fluent_t &column);

};

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_SQLSERVER_GRAMMAR_H_
