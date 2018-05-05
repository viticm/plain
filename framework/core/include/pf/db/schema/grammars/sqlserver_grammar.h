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

   SqlserverGrammar() {}
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
   virtual std::string compile_column_listing(const std::string &table) const {
     return "select col.name from sys.columns as col \
join sys.objects as obj on col.object_id = obj.object_id \
where obj.type = 'U' and obj.name = '" + table + "'";
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

   //The possible column modifiers.
   std::vector<std::string> modifiers_;

   //The possible column serials.
   std::vector<std::string> serials_;

 protected:

   //Create the column definition for a char type.
   std::string type_char(const std::string &column) const;

   //Create the column definition for a string type.
   std::string type_string(const std::string &column) const;

   //Create the column definition for a text type.
   std::string type_text(const std::string &column) const;

   //Create the column definition for a medium text type.
   std::string type_medium_text(const std::string &column) const;

   //Create the column definition for a long text type.
   std::string type_long_text(const std::string &column) const;

   //Create the column definition for a big integer type.
   std::string type_big_integer(const std::string &column) const;

   //Create the column definition for an integer type.
   std::string type_integer(const std::string &column) const;

   //Create the column definition for a medium integer type.
   std::string type_medium_integer(const std::string &column) const;

   //Create the column definition for a tiny integer type.
   std::string type_tiny_integer(const std::string &column) const;

   //Create the column definition for a small integer type.
   std::string type_small_integer(const std::string &column) const;

   //Create the column definition for a float type.
   std::string type_float(const std::string &column) const;

   //Create the column definition for a double type.
   std::string type_double(const std::string &column) const;

   //Create the column definition for a decimal type.
   std::string type_decimal(const std::string &column) const; 

   //Create the column definition for a boolean type.
   std::string type_boolean(const std::string &column) const;

   //Create the column definition for an enum type.
   std::string type_enum(const std::string &column) const;

   //Create the column definition for a json type.
   std::string type_json(const std::string &column) const;

   //Create the column definition for a jsonb type.
   std::string type_jsonb(const std::string &column) const; 

   //Create the column definition for a date type.
   std::string type_date(const std::string &column) const; 

   //Create the column definition for a date-time type.
   std::string type_date_time(const std::string &column) const;

   //Create the column definition for a date-time type.
   std::string type_date_time_tz(const std::string &column) const;

   //Create the column definition for a time type.
   std::string type_time(const std::string &column) const;

   //Create the column definition for a time type.
   std::string type_time_tz(const std::string &column) const;

   //Create the column definition for a timestamp type.
   std::string type_timestamp(const std::string &column) const;

   //Create the column definition for a binary type.
   std::string type_blob(const std::string &column) const;

   //Create the column definition for a uuid type.
   std::string type_uuid(const std::string &column) const;

   //Create the column definition for an IP address type.
   std::string type_ip_address(const std::string &column) const;

   //Create the column definition for a MAC address type.
   std::string type_mac_address(const std::string &column) const;

   //Get the SQL for a generated virtual column modifier.
   std::string modify_virtual_as(Blueprint *blueprint, const std::string &column);

   //Get the SQL for a generated stored column modifier.
   std::string modify_stored_as(Blueprint *blueprint, const std::string &column);

   //Get the SQL for an unsigned column modifier.
   std::string modify_unsigned(Blueprint *blueprint, const std::string &column);

   //Get the SQL for a character set column modifier.
   std::string modify_character(Blueprint *blueprint, const std::string &column);

   //Get the SQL for a collation column modifier.
   std::string modify_collation(Blueprint *blueprint, const std::string &column);

   //Get the SQL for a default column modifier.
   std::string modify_default(Blueprint *blueprint, const std::string &column);

   //Get the SQL for an auto-increment column modifier.
   std::string modify_increment(Blueprint *blueprint, const std::string &column);

   //Wrap a single string in keyword identifiers.
   virtual std::string wrap_value(const variable_t &value);

}

} //namespace grammars

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_GRAMMARS_SQLSERVER_GRAMMAR_H_
