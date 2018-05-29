#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/schema/blueprint.h"
#include "pf/db/schema/grammars/sqlite_grammar.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_db::schema::grammars;

SqliteGrammar::SqliteGrammar() {

  modifiers_ = {
    "nullable", "default", "increment",
  };

  serials_ = {
    "big_integer", "integer", "medium_integer", "small_integer", "tiny_integer",
  };
}

//Compile the query to determine the list of columns.
std::string SqliteGrammar::compile_column_listing(const std::string &table) {
  return "pragma table_info(" + wrap_table(str_replace(".", "__", table)) + ")";
}

//Compile a create table command.
std::string SqliteGrammar::compile_create(Blueprint *blueprint, 
                                          fluent_t &command, 
                                          ConnectionInterface *connection) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "%s table %s (%s%s%s)",
           blueprint->temporary_ ? "create temporary" : "create",
           wrap_table(blueprint).c_str(),
           implode(", ", get_columns(blueprint)).c_str(),
           add_foreign_keys(blueprint).c_str(),
           add_primary_keys(blueprint).c_str());
  return temp;
}

//Get the foreign key syntax for a table creation statement.
std::string SqliteGrammar::add_foreign_keys(Blueprint *blueprint) {
  auto foreigns = get_commands_by_name(blueprint, "foreign");
  return collect(foreigns).reduce([this](std::string &sql, fluent_t &foreign) {
    // Once we have all the foreign key commands for the table creation statement
    // we'll loop through each of them and add them to the create table SQL we 
    // are building, since SQLite needs foreign keys on the tables creation.
    sql += get_foreign_key(foreign);

    if (!empty(foreign["on_delete"]))
      sql += " on delete " + foreign["on_delete"].data;

    // If this foreign key specifies the action to be taken on update we will add 
    // that to the statement here. We'll append it to this SQL and then return 
    // the SQL so we can keep adding any other foreign consraints onto this.
    if (!empty(foreign["on_update"]))
      sql += " on update " + foreign["on_update"].data;

    return sql;    
  }, "");
}

//Get the SQL for the foreign key.
std::string SqliteGrammar::get_foreign_key(fluent_t &foreign) {
  // We need to columnize the columns that the foreign key is being defined for
  // so that it is a properly formatted list. Once we have done this, we can 
  // return the foreign key SQL declaration to the calling method for use.
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           ", foreign key(%s) references %s(%s)",
           columnize(foreign.columns).c_str(),
           wrap_table(foreign["on"]).c_str(),
           columnize(foreign.references).c_str());
  return temp;
}

//Get the primary key syntax for a table creation statement.
std::string SqliteGrammar::add_primary_keys(Blueprint *blueprint) {
  auto primary = get_command_by_name(blueprint, "primary");
  if (primary["name"] != "")
    return ", primary key (" + columnize(primary.columns) + ")";
  return "";
}

//Compile an add column command.
std::string SqliteGrammar::compile_add(Blueprint *blueprint, fluent_t &command) {
  auto _columns = prefix_array("add column", get_columns(blueprint));
  std::vector<std::string> columns;
  for (auto &column : _columns) {
    columns.emplace_back(
        "alter table " + wrap_table(blueprint) + " " + column.data);
  }
  return implode("; ", columns);
}

//Compile a unique key command.
std::string SqliteGrammar::compile_unique(
    Blueprint *blueprint, fluent_t &command) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "create unique index %s on %s (%s)",
           wrap(command["index"]).c_str(),
           wrap_table(blueprint).c_str(),
           columnize(command.columns).c_str());
  return temp;
}

//Compile a unique key command.
std::string SqliteGrammar::compile_index(
    Blueprint *blueprint, fluent_t &command) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "create index %s on %s (%s)",
           wrap(command["index"]).c_str(),
           wrap_table(blueprint).c_str(),
           columnize(command.columns).c_str());
  return temp;
}

//Compile a drop column command.
std::string SqliteGrammar::compile_drop_column(
   Blueprint *, fluent_t &) {
  return "";
}

//Get the SQL for a default column modifier.
std::string SqliteGrammar::modify_default(
    Blueprint *blueprint, fluent_t &column) {
  if (!empty(column["default"]))
    return " default " + get_default_value(column["default"]);
  return "";
}

//Get the SQL for an auto-increment column modifier.
std::string SqliteGrammar::modify_increment(
    Blueprint *blueprint, fluent_t &column) {
  if (in_array(column["type"].data, serials_) && 
      column["auto_increment"] == true) {
    return " primary key autoincrement";
  }
  return "";
}
