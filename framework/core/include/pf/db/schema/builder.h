/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id builder.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/03/09 11:03
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_BUILDER_H_
#define PF_DB_SCHEMA_BUILDER_H_

#include "pf/db/schema/config.h"
#include "pf/db/schema/blueprint.h"

namespace pf_db {

namespace schema {

class PF_API Builder {

 public:
   Builder(ConnectionInterface *connection) : connection_{connection} {};
   virtual ~Builder() {};

 public:
   using closure_t = 
     std::function<Blueprint *(const std::string &, Blueprint::closure_t)>;

 public:

   //The default string length for migrations.
   static int32_t default_string_length_;

 public:
   
   //Set the default string length for migrations.
   void default_string_length(int32_t length) {
     default_string_length_ = length;
   };

   //Determine if the given table exists.
   virtual bool has_table(const std::string &table);

   //Determine if the given table has a given column.
   bool has_column(const std::string &table, const std::string &column);

   //Determine if the given table has given columns.
   bool has_columns(const std::string &table, 
                    const std::vector<std::string> &columns);

   //Get the data type for the given column name.
   std::string get_column_type(
       const std::string &table, const std::string &column);

   //Get the column listing for a given table.
   virtual std::vector<std::string> get_column_listing(const std::string &table);

   //Modify a table on the schema.
   void table(const std::string &_table, Blueprint::closure_t callback) {
     std::unique_ptr<Blueprint> blueprint(create_blueprint(_table, callback));
     build(blueprint);
   };

   //Create a new table on the schema.
   void create(const std::string &table, Blueprint::closure_t callback); 

   //Drop a table from the schema.
   void drop(const std::string &table);

   //Drop a table from the schema if it exists.
   void drop_if_exists(const std::string &table);

   //Rename a table on the schema.
   void rename(const std::string &from, const std::string &to);

   //Enable foreign key constraints.
   bool enable_foreign_key_constraints();

   //Disable foreign key constraints.
   bool disable_foreign_key_constraints();

 protected:

   //The database connection instance.
   ConnectionInterface *connection_;

   //The schema grammar instance.
   grammars::Grammar *grammar_;

   //The Blueprint resolver callback.
   closure_t resolver_;

 protected:

   //Execute the blueprint to build / modify the table.
   void build(std::unique_ptr<Blueprint> &blueprint);

   //Create a new command set with a Closure.
   Blueprint *create_blueprint(
       const std::string &table, Blueprint::closure_t callback = nullptr);

   //Get the database connection instance.
   ConnectionInterface *get_connection() {
     return connection_;
   }

   //Set the Schema Blueprint resolver callback.
   void blueprint_resolver(closure_t resolver) {
     resolver_ = resolver;
   }

};

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_BUILDER_H_
