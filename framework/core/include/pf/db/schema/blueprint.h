/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id blueprint.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/03/08 14:37
 * @uses your description
*/
#ifndef PF_DB_SCHEAM_BLUEPRINT_H_
#define PF_DB_SCHEAM_BLUEPRINT_H_

#include "pf/db/schema/config.h"
#include "pf/db/schema/builder.h"

namespace pf_db {

namespace schema {

class PF_API Blueprint {

 public:

   using closure_t = std::function<void(Blueprint *)>;
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using command_t = db_schema_command_t;

 public:

   //The construct.
   Blueprint(const std::string &table, closure_t callback = nullptr);

   //The destruct.
   ~Blueprint();

 public:

   //The storage engine that should be used for the table.
   std::string engine_;

   //The default character set that should be used for the table.
   std::string charset_;

   //The collation that should be used for the table.
   std::string collation_;

   //Whether to make the table temporary.
   bool temporary_;

 public:

   //Get the raw SQL statements for the blueprint. 
   std::vector<std::string> to_sql(
       Connection *connection, grammars::Grammar *grammar);

   //Execute the blueprint against the database.
   void build(Connection *connection, grammars::Grammar *grammar);

   //Indicate that the table needs to be created.
   void create();

   //Indicate that the table needs to be temporary.
   void temporary();

   //Indicate that the table should be dropped.
   void drop() {
     add_command("drop", params);
   };

   //Indicate that the table should be dropped if it exists.
   void drop_ifexists() {
     variable_set_t params;
     add_command("drop_ifexists");
   };
   //Indicate that the given columns should be dropped.
   void drop_column(const std::vector<std::string> &columns) {
     variable_set_t params;
     for (const std::string &column : columns)
       params[column] = column;
     add_command("drop_column", params);
   };

   //Indicate that the given columns should be renamed.
   void rename_column(const std::vector &from, const std::string &to) {
     variable_array_t params;
     params["from"] = from;
     params["to"] = to;
     add_command("rename_column", params);
   };

   //Indicate that the given primary key should be dropped.
   void drop_primary(const std::string &index = "") {
     drop_index_command("drop_primary", "primary", index);
   };

   //Indicate that the given unique key should be dropped.
   void drop_unique(const std::string &index) {
     drop_index_command("drop_primary", "unique", index);
   };

   //Indicate that the given index should be dropped.
   void drop_index(const std::string &index) {
     drop_index_command("drop_primary", "index", index);
   };

   //Indicate that the given foreign key should be dropped.
   void drop_foreign(const std::string &index) {
     drop_index_command("drop_primary", "foreign", index);
   };

   //Indicate that the timestamp columns should be dropped.
   void drop_timestamps() {
     drop_column({"created_at", "updated_at"});
   };

   //Indicate that the timestamp columns should be dropped.
   void drop_timestamps_tz() {
     drop_timestamps();
   };

   //Indicate that the soft delete column should be dropped.
   void drop_soft_deletes() {
     drop_column({"deleted_at"});
   };

   //Indicate that the soft delete column should be dropped.
   void drop_soft_deletes_tz() {
     drop_soft_deletes();
   };

   //Indicate that the remember token column should be dropped.
   void drop_remember_token() {
     drop_column({"remember_token"});
   };

   //Rename the table to a given name.
   void rename(const std::string &to) {
     variable_set_t params;
     params["to"] = to;
     add_command("rename", params);
   };

   //Specify the primary key(s) for the table.
   void primary(const std::vector<std::vector> columns,
                const std::string &name = "",
                const std::string &algorithm = "") {
     index_command("primary", columns, name, algorithm);
   };

   //Specify a unique index for the table.
   void unique(const std::vector<std::vector> columns,
               const std::string &name = "",
               const std::string &algorithm = "") {
     index_command("unique", columns, name, algorithm);
   };

   //Specify an index for the table.
   void index(const std::vector<std::vector> columns,
              const std::string &name = "",
              const std::string &algorithm = "") {
     index_command("index", columns, name, algorithm);
   };

   //Specify a foreign key for the table.
   void foreign(const std::vector<std::vector> columns, 
                const std::string &name = "") {
     index_command("foreign", columns, name);
   };

   //Create a new auto-incrementing integer (4-byte) column on the table.
   void increments(const std::string &column) {
     unsigned_integer(column, true);
   };

   //Create a new auto-incrementing tiny integer (1-byte) column on the table.
   void tiny_increments(const std::string &column) {
     unsigned_tiny_integer(column, true);
   };

   //Create a new auto-incrementing small integer (2-byte) column on the table.
   void small_increments(const std::string &column) {
     unsigned_small_integer(column, true);
   };

   //Create a new auto-incrementing medium integer (3-byte) column on the table.
   void medium_increments(const std::string &column) {
     unsigned_medium_integer(column, true);
   };

   //Create a new auto-incrementing big integer (8-byte) column on the table.
   void big_increments(const std::string &column) {
     unsigned_big_integer(column, true);
   };

   //Create a new char column on the table.
   void _char(const std::string &column, int32_t length = -1) {
     int32_t rlength = -1 == length ? Builder::default_string_length_ : length;
     variable_set_t params = {{"length", rlength}, };
     add_column("char", column, params);     
   };

   //Create a new string column on the table.
   void string(const std::string &column, int32_t length = -1) {
     int32_t rlength = -1 == length ? Builder::default_string_length_ : length;
     variable_set_t params = {{"length", rlength}, };
     add_column("string", column, params);
   };

   //Create a new text column on the table.
   void text(const std::string &column) {
     add_column("text", column);
   };

   //Create a new medium text column on the table.
   void medium_text(const std::string &column) {
     add_column("medium_text", column);
   };

   //Create a new long text column on the table.
   void long_text(const std::string &column) {
     add_column("long_text", column);
   };

   //Create a new integer (4-byte) column on the table.
   void integer(const std::string &column, 
                bool auto_increment = false, 
                bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     add_column("integer", column, params);
   };

   //Create a new tiny integer (1-byte) column on the table.
   void tiny_integer(const std::string &column, 
                     bool auto_increment = false, 
                     bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     add_column("tiny_integer", column, params);
   };

   //Create a new small integer (2-byte) column on the table.
   void small_integer(const std::string &column, 
                      bool auto_increment = false, 
                      bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     add_column("small_integer", column, params);
   };

   //Create a new medium integer (3-byte) column on the table.
   void medium_integer(const std::string &column, 
                       bool auto_increment = false, 
                       bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     add_column("medium_integer", column, params);
   };
  
   //Create a new big integer (8-byte) column on the table.
   void big_integer(const std::string &column, 
                    bool auto_increment = false, 
                    bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     add_column("big_integer", column, params);
   };
 
   //Create a new unsigned integer (4-byte) column on the table.
   void unsigned_integer(const std::string &column, 
                         bool auto_increment = false) {
     integer(column, auto_increment, true);
   };

   //Create a new unsigned tiny integer (1-byte) column on the table.
   void unsigned_tiny_integer(const std::string &column, 
                              bool auto_increment = false) {
     tiny_integer(column, auto_increment, true);
   };

   //Create a new unsigned small integer (2-byte) column on the table.
   void unsigned_small_integer(const std::string &column, 
                               bool auto_increment = false) {
     small_integer(column, auto_increment, true);
   };

   //Create a new unsigned medium integer (3-byte) column on the table.
   void unsigned_medium_integer(const std::string &column, 
                                bool auto_increment = false) {
     medium_integer(column, auto_increment, true);
   };

   //Create a new unsigned big integer (8-byte) column on the table.
   void unsigned_big_integer(const std::string &column, 
                             bool auto_increment = false) {
     big_integer(column, auto_increment, true);
   };

   //Create a new float column on the table.
   void _float(const std::string &column, int32_t total = 8, places = 2) {
     variable_set_t params;
     params["total"] = total;
     params["places"] = places;
     add_column("float", column, params);
   };

   //Create a new double column on the table.
   void _double(const std::string &column, int32_t total = -1, places = -1) {
     variable_set_t params;
     params["total"] = total;
     params["places"] = places;
     add_column("double", column, params);
   };

   //Create a new decimal column on the table.
   void decimal(const std::string &column, int32_t total = 8, places = 2) {
     variable_set_t params;
     params["total"] = total;
     params["places"] = places;
     add_column("decimal", column, params);
   };

   //Create a new boolean column on the table.
   void boolean(const std::string &column) {
     add_column("boolean", column);
   };

   //Create a new enum column on the table.
   void _enum(const std::string &column, const std::vector<int32_t> &allowed);

   //Create a new json column on the table.
   void json(const std::string &column) {
     add_column("json", column);
   };

   //Create a new jsonb column on the table.
   void jsonb(const std::string &column) {
     add_column("jsonb", column);
   };

   //Create a new date column on the table.
   void date(const std::string &column) {
     add_column("date", column);
   };

   //Create a new date-time column on the table.
   void date_time(const std::string &column) {
     add_column("date_time", column);
   };

   //Create a new date-time column (with time zone) on the table.
   void date_time_tz(const std::string &column) {
     add_column("date_time_tz", column);
   };

   //Create a new time column on the table.
   void time(const std::string &column) {
     add_column("time", column);
   };

   //Create a new time column (with time zone) on the table.
   void time_tz(const std::string &column) {
     add_column("time_tz", column);
   };

   //Create a new timestamp column on the table.
   void timestamp(const std::string &column) {
     add_column("timestamp", column);
   };

   //Create a new timestamp (with time zone) column on the table.
   void timestamp_tz(const std::string &column) {
     add_column("timestamp_tz", column);
   };

   //Add nullable creation and update timestamps to the table.
   void timestamps(const std::string &column) {
     
   };

   //Add nullable creation and update timestamps to the table.
   void nullable_timestamps();

   //Add creation and update timestampTz columns to the table.
   void timestamps_tz();

   //Add a "deleted at" timestamp for the table.
   void soft_deletes(const std::string &column = "deleted_at");

   //Add a "deleted at" timestampTz for the table.
   void soft_deletes_tz();

   //Create a new binary column on the table.
   void binary(const std::string &column);

   //Create a new uuid column on the table.
   void uuid(const std::string &column);

   //Create a new IP address column on the table.
   void ip_address(const std::string &column);

   //Create a new MAC address column on the table.
   void mac_address(const std::string &column);

   //Add the proper columns for a polymorphic table.
   void morphs(const std::string &column, const std::string &index_name = "");

   //Add nullable columns for a polymorphic table. 
   void nullable_morphs(
       const std::string &column, const std::string &index_name = "");

   //Adds the `remember_token` column to the table.
   void remember_token();

   //Add a new column to the blueprint.
   void add_column(const std::string &type, 
                   const std::string &name, 
                   variable_set_t &param);

   //Add a new column to the blueprint.
   void add_column(const std::string &type, 
                   const std::string &name) {
     variable_set_t param;
     add_column(type, name, param);
   };

   //Remove a column from the schema blueprint.
   void remove_column(const std::string &name);

   //Get the table the blueprint describes.
   std::string get_table() const {
     return table_;
   };

   //Get the columns on the blueprint.
   variable_array_t get_columns() {
     return columns_;
   }

   //Get the commands on the blueprint.
   std::vector<std::string> get_commands() {
     return commands_;
   }
   
   //Get the columns on the blueprint that should be added.
   std::vector<std::string> get_added_columns();

   //Get the columns on the blueprint that should be changed.
   std::vector<std::string> get_changed_columns();

 protected:

   //The table the blueprint describes.
   std::string table_;

   //The columns that should be added to the table.
   variable_array_t columns_;

   //The commands that should be run for the table.
   std::vector<command_t> commands_;

 protected:

   //Add the commands that are implied by the blueprint's state.
   void add_implied_commands();

   //Add the index commands fluently specified on columns.
   void add_fluent_indexes();

   //Determine if the blueprint has a create command.
   bool creating();

   //Add a new index command to the blueprint.
   void index_command(const std::string &type, 
                      const std::vector<std::string> &columns, 
                      const std::string &index, 
                      const std::string &algorithm = "");

   //Create a new drop index command on the blueprint.
   void drop_index_command(const std::string &command, 
                           const std::string &type, 
                           const std::string &index);

   //Create a default index name for the table.
   void create_index_name(const std::string &type, const std::string &index);

   //Add a new command to the blueprint.
   command_t add_command(const std::string &name, variable_set_t &param) {
     command_t command;
     command.name = name;
     command.params = param;
     commands_.emplace_back(command);
     return command;
   };

   //Add a new command to the blueprint.
   command_t add_command(const std::string &name) {
     variable_set_t params;
     return add_command(name, params);
   };

   //Create a new Fluent command.
   command_t create_command(const std::string &name, variable_set_t &param);

}

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEAM_BLUEPRINT_H_
