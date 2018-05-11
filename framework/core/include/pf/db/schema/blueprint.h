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

namespace pf_db {

namespace schema {

class PF_API Blueprint {

 public:

   using closure_t = std::function<void(Blueprint *)>;
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using fluent_t = db_schema_fluent_t;

 public:

   //The construct.
   Blueprint(const std::string &table, closure_t callback = nullptr) : 
     temporary_{false}, table_{table} {
     if (!is_null(callback)) 
       callback(this);
   }

   //The destruct.
   ~Blueprint() {};

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
       ConnectionInterface *connection, grammars::Grammar *grammar);

   //Execute the blueprint against the database.
   void build(ConnectionInterface *connection, grammars::Grammar *grammar);

   //Indicate that the table needs to be created.
   void create() {
     add_command("create");
   }

   //Reset blueprint values.
   void clear();

   //Indicate that the table needs to be temporary.
   void temporary() {
     temporary_ = true;
   }

   //Indicate that the table should be dropped.
   void drop() {
     add_command("drop");
   }

   //Indicate that the table should be dropped if it exists.
   void drop_if_exists() {
     variable_set_t params;
     add_command("drop_if_exists");
   }
   //Indicate that the given columns should be dropped.
   void drop_column(const std::vector<std::string> &columns) {
     variable_set_t params;
     for (const std::string &column : columns)
       params[column] = column;
     add_command("drop_column", params);
   }

   //Indicate that the given columns should be renamed.
   void rename_column(const std::string &from, const std::string &to) {
     variable_set_t params;
     params["from"] = from;
     params["to"] = to;
     add_command("rename_column", params);
   }

   //Indicate that the given primary key should be dropped.
   void drop_primary(const std::string &_index = "") {
     drop_index_command("drop_primary", "primary", _index);
   }

   //Indicate that the given unique key should be dropped.
   void drop_unique(const std::string &_index) {
     drop_index_command("drop_primary", "unique", _index);
   }

   //Indicate that the given index should be dropped.
   void drop_index(const std::string &_index) {
     drop_index_command("drop_primary", "index", _index);
   }

   //Indicate that the given foreign key should be dropped.
   void drop_foreign(const std::string &_index) {
     drop_index_command("drop_primary", "foreign", _index);
   }

   //Indicate that the timestamp columns should be dropped.
   void drop_timestamps() {
     drop_column({"created_at", "updated_at"});
   }

   //Indicate that the timestamp columns should be dropped.
   void drop_timestamps_tz() {
     drop_timestamps();
   }

   //Indicate that the soft delete column should be dropped.
   void drop_soft_deletes() {
     drop_column({"deleted_at"});
   }

   //Indicate that the soft delete column should be dropped.
   void drop_soft_deletes_tz() {
     drop_soft_deletes();
   }

   //Indicate that the remember token column should be dropped.
   void drop_remember_token() {
     drop_column({"remember_token"});
   }

   //Rename the table to a given name.
   void rename(const std::string &to) {
     variable_set_t params;
     params["to"] = to;
     add_command("rename", params);
   }

   //Specify the primary key(s) for the table.
   void primary(const std::vector<std::string> &columns,
                const std::string &name = "",
                const std::string &algorithm = "") {
     index_command("primary", columns, name, algorithm);
   }

   //Specify a unique index for the table.
   void unique(const std::vector<std::string> &columns,
               const std::string &name = "",
               const std::string &algorithm = "") {
     index_command("unique", columns, name, algorithm);
   }

   //Specify an index for the table.
   void index(const std::vector<std::string> &columns,
              const std::string &name = "",
              const std::string &algorithm = "") {
     index_command("index", columns, name, algorithm);
   }

   //Specify a foreign key for the table.
   void foreign(const std::vector<std::string> &columns, 
                const std::string &name = "") {
     index_command("foreign", columns, name);
   }

   //Create a new auto-incrementing integer (4-byte) column on the table.
   fluent_t &increments(const std::string &column) {
     return unsigned_integer(column, true);
   }

   //Create a new auto-incrementing tiny integer (1-byte) column on the table.
   fluent_t &tiny_increments(const std::string &column) {
     return unsigned_tiny_integer(column, true);
   }

   //Create a new auto-incrementing small integer (2-byte) column on the table.
   fluent_t &small_increments(const std::string &column) {
     return unsigned_small_integer(column, true);
   }

   //Create a new auto-incrementing medium integer (3-byte) column on the table.
   fluent_t &medium_increments(const std::string &column) {
     return unsigned_medium_integer(column, true);
   }

   //Create a new auto-incrementing big integer (8-byte) column on the table.
   fluent_t &big_increments(const std::string &column) {
     return unsigned_big_integer(column, true);
   }

   //Create a new char column on the table.
   fluent_t &_char(const std::string &column, int32_t length = -1);

   //Create a new string column on the table.
   fluent_t &string(const std::string &column, int32_t length = -1);

   //Create a new text column on the table.
   fluent_t &text(const std::string &column) {
     return add_column("text", column);
   }

   //Create a new medium text column on the table.
   fluent_t &medium_text(const std::string &column) {
     return add_column("medium_text", column);
   }

   //Create a new long text column on the table.
   fluent_t &long_text(const std::string &column) {
     return add_column("long_text", column);
   }

   //Create a new integer (4-byte) column on the table.
   fluent_t &integer(const std::string &column, 
                     bool auto_increment = false, 
                     bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     return add_column("integer", column, params);
   }

   //Create a new tiny integer (1-byte) column on the table.
   fluent_t &tiny_integer(const std::string &column, 
                          bool auto_increment = false, 
                          bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     return add_column("tiny_integer", column, params);
   }

   //Create a new small integer (2-byte) column on the table.
   fluent_t &small_integer(const std::string &column, 
                           bool auto_increment = false, 
                           bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     return add_column("small_integer", column, params);
   }

   //Create a new medium integer (3-byte) column on the table.
   fluent_t &medium_integer(const std::string &column, 
                            bool auto_increment = false, 
                            bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     return add_column("medium_integer", column, params);
   }
  
   //Create a new big integer (8-byte) column on the table.
   fluent_t &big_integer(const std::string &column, 
                         bool auto_increment = false, 
                         bool _unsigned = false) {
     variable_set_t params;
     params["auto_increment"] = auto_increment;
     params["unsigned"] = _unsigned;
     return add_column("big_integer", column, params);
   }
 
   //Create a new unsigned integer (4-byte) column on the table.
   fluent_t &unsigned_integer(const std::string &column, 
                              bool auto_increment = false) {
     return integer(column, auto_increment, true);
   }

   //Create a new unsigned tiny integer (1-byte) column on the table.
   fluent_t &unsigned_tiny_integer(const std::string &column, 
                                   bool auto_increment = false) {
     return tiny_integer(column, auto_increment, true);
   }

   //Create a new unsigned small integer (2-byte) column on the table.
   fluent_t &unsigned_small_integer(const std::string &column, 
                                    bool auto_increment = false) {
     return small_integer(column, auto_increment, true);
   }

   //Create a new unsigned medium integer (3-byte) column on the table.
   fluent_t &unsigned_medium_integer(const std::string &column, 
                                     bool auto_increment = false) {
     return medium_integer(column, auto_increment, true);
   }

   //Create a new unsigned big integer (8-byte) column on the table.
   fluent_t &unsigned_big_integer(const std::string &column, 
                                  bool auto_increment = false) {
     return big_integer(column, auto_increment, true);
   }

   //Create a new float column on the table.
   fluent_t &_float(
       const std::string &column, int32_t total = 8, int32_t places = 2) {
     variable_set_t params;
     params["total"] = total;
     params["places"] = places;
     return add_column("float", column, params);
   }

   //Create a new double column on the table.
   fluent_t &_double(
       const std::string &column, int32_t total = -1, int32_t places = -1) {
     variable_set_t params;
     params["total"] = total;
     params["places"] = places;
     return add_column("double", column, params);
   }

   //Create a new decimal column on the table.
   fluent_t &decimal(
       const std::string &column, int32_t total = 8, int32_t places = 2) {
     variable_set_t params;
     params["total"] = total;
     params["places"] = places;
     return add_column("decimal", column, params);
   }

   //Create a new boolean column on the table.
   fluent_t &boolean(const std::string &column) {
     return add_column("boolean", column);
   }

   //Create a new enum column on the table.
   fluent_t &_enum(const std::string &column, 
                   const std::vector<int32_t> &allowed) {
     return add_column("enum", column, allowed);
   }

   //Create a new json column on the table.
   fluent_t &json(const std::string &column) {
     return add_column("json", column);
   }

   //Create a new jsonb column on the table.
   fluent_t &jsonb(const std::string &column) {
     return add_column("jsonb", column);
   }

   //Create a new date column on the table.
   fluent_t &date(const std::string &column) {
     return add_column("date", column);
   }

   //Create a new date-time column on the table.
   fluent_t &date_time(const std::string &column) {
     return add_column("date_time", column);
   }

   //Create a new date-time column (with time zone) on the table.
   fluent_t &date_time_tz(const std::string &column) {
     return add_column("date_time_tz", column);
   }

   //Create a new time column on the table.
   fluent_t &time(const std::string &column) {
     return add_column("time", column);
   }

   //Create a new time column (with time zone) on the table.
   fluent_t &time_tz(const std::string &column) {
     return add_column("time_tz", column);
   }

   //Create a new timestamp column on the table.
   fluent_t &timestamp(const std::string &column) {
     return add_column("timestamp", column);
   }

   //Create a new timestamp (with time zone) column on the table.
   fluent_t &timestamp_tz(const std::string &column) {
     return add_column("timestamp_tz", column);
   }

   //Add nullable creation and update timestamps to the table.
   void timestamps() {
     timestamp("created_at").nullable();
     timestamp("updated_at").nullable();
   }

   //Add nullable creation and update timestamps to the table.
   void nullable_timestamps() {
     timestamps();
   }

   //Add creation and update timestampTz columns to the table.
   void timestamps_tz() {
     timestamp_tz("created_at").nullable();
     timestamp_tz("updated_at").nullable();
   }

   //Add a "deleted at" timestamp for the table.
   fluent_t &soft_deletes(const std::string &column = "deleted_at") {
     return timestamp(column).nullable();
   }

   //Add a "deleted at" timestampTz for the table.
   fluent_t &soft_deletes_tz() {
     return timestamp_tz("deleted_at").nullable();
   }

   //Create a new binary column on the table.
   fluent_t &binary(const std::string &column) {
     return add_column("binary", column);
   }

   //Create a new uuid column on the table.
   fluent_t &uuid(const std::string &column) {
     return add_column("uuid", column);
   }

   //Create a new IP address column on the table.
   fluent_t &ip_address(const std::string &column) {
     return add_column("ip_address", column);
   }

   //Create a new MAC address column on the table.
   fluent_t &mac_address(const std::string &column) {
     return add_column("mac_address", column);
   }

   //Add the proper columns for a polymorphic table.
   void morphs(const std::string &name, const std::string &index_name = "") {
     unsigned_integer(name + "_id");
     string(name + "_type");
     index({name + "_id", name + "_type"}, index_name);
   }

   //Add nullable columns for a polymorphic table. 
   void nullable_morphs(
       const std::string &name, const std::string &index_name = "") {
     unsigned_integer(name + "_id").nullable();
     string(name + "_type");
     index({name + "_id", name + "_type"}, index_name);
   }

   //Adds the `remember_token` column to the table.
   fluent_t &remember_token() {
     return string("remember_token", 100).nullable();
   }

   //Add a new column to the blueprint.
   fluent_t &add_column(const std::string &type, 
                        const std::string &name, 
                        variable_set_t &param) {
     fluent_t column;
     for (auto it = param.begin(); it != param.end(); ++it)
       column.items[it->first] = it->second;
     column.items["type"] = type;
     column.items["name"] = name;
     columns_.emplace_back(column);
     return columns_[columns_.size() - 1];
   }

   //Add a new column to the blueprint.
   fluent_t &add_column(const std::string &type, 
                        const std::string &name, 
                        const std::vector<int32_t> allowed) {
     fluent_t column;
     column.items["type"] = type;
     column.items["name"] = name;
     column.allowed = allowed;
     columns_.emplace_back(column);
     return columns_[columns_.size() - 1];
   }

   //Add a new column to the blueprint.
   fluent_t &add_column(const std::string &type, 
                        const std::string &name) {
     variable_set_t param;
     return add_column(type, name, param);
   }

   //Remove a column from the schema blueprint.
   Blueprint &remove_column(const std::string &name) {
     auto find_it = columns_.end();
     for (auto it = columns_.begin(); it != columns_.end(); ++it) {
       if ((*it).items["name"] == name) find_it = it;
     }
     if (find_it != columns_.end()) columns_.erase(find_it);
     return *this;
   }

   //Get the table the blueprint describes.
   std::string get_table() const {
     return table_;
   }

   //Get the columns on the blueprint.
   std::vector<fluent_t> &get_columns() {
     return columns_;
   }

   //Get the commands on the blueprint.
   std::vector<fluent_t> &get_commands() {
     return commands_;
   }
   
   //Get the columns on the blueprint that should be added.
   std::vector<fluent_t> get_added_columns();

   //Get the columns on the blueprint that should be changed.
   std::vector<fluent_t> get_changed_columns();

   //Pass the query to a given callback.
   Blueprint &tap(closure_t callback) {
     callback(this);
     return *this;
   }

 protected:

   //The table the blueprint describes.
   std::string table_;

   //The columns that should be added to the table.
   std::vector<fluent_t> columns_;

   //The commands that should be run for the table.
   std::vector<fluent_t> commands_;

 protected:

   //Add the commands that are implied by the blueprint's state.
   void add_implied_commands();

   //Add the index commands fluently specified on columns.
   void add_fluent_indexes();

   //Determine if the blueprint has a create command.
   bool creating();

   //Add a new index command to the blueprint.
   fluent_t &index_command(const std::string &type, 
                           const std::vector<std::string> &columns, 
                           const std::string &_index = "", 
                           const std::string &algorithm = "") {
     std::string temp{_index};
     if (temp == "") temp = create_index_name(type, columns);
     variable_set_t param = {{"index", temp}, {"algorithm", algorithm}};
     return add_command(type, param, columns);
   }

   //Create a new drop index command on the blueprint.
   fluent_t &drop_index_command(const std::string &command, 
                                const std::string &, 
                                const std::string &_index) {
     std::vector<std::string> columns;
     return index_command(command, columns, _index);
   }

   //Create a new drop index command on the blueprint.
   fluent_t &drop_index_command(const std::string &command, 
                                const std::string &type, 
                                const std::vector<std::string> &columns) {
     std::string _index = create_index_name(type, columns);
     return index_command(command, columns, _index);
   }

   //Create a default index name for the table.
   std::string create_index_name(const std::string &type, 
                                 const std::vector<std::string> &columns);

   //Add a new command to the blueprint.
   fluent_t &add_command(const std::string &name, 
                         variable_set_t &param, 
                         const std::vector<std::string> columns = {}) {
     fluent_t command;
     command["name"] = name;
     command.columns = columns;
     for (auto it = param.begin(); it != param.end(); ++it)
       command[it->first] = it->second;
     commands_.emplace_back(command);
     return commands_[commands_.size() - 1];
   }

   //Add a new command to the blueprint.
   fluent_t &add_command(const std::string &name) {
     variable_set_t params;
     return add_command(name, params);
   }

   //Create a new Fluent command.
   fluent_t create_command(const std::string &name, variable_set_t &param) {
     fluent_t r;
     r["name"] = name;
     for (auto it = param.begin(); it != param.end(); ++it)
       r[it->first] = it->second;
     return r;
   };

   //Create a new Fluent command.
   fluent_t create_command(const std::string &name) {
     variable_set_t param;
     return create_command(name, param);
   }

};

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEAM_BLUEPRINT_H_
