#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/schema/blueprint.h"
#include "pf/db/schema/grammars/sqlserver_grammar.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_db::schema::grammars;

SqlserverGrammar::SqlserverGrammar() {
  
  modifiers_ = {
    "increment", "collate", "nullable", "default",
  };

  serials_ = {
    "tiny_integer", "small_integer", "medium_integer", "integer", "big_integer",
  };

}

//Compile a create table command.
std::string SqlserverGrammar::compile_create(Blueprint *blueprint, 
                                             fluent_t &command, 
                                             ConnectionInterface *connection) {
  UNUSED(command); UNUSED(connection);
  auto columns = implode(", ", get_columns(blueprint));
  return "create table " + wrap_table(blueprint) + " (" + columns + ")";
}

//Compile an add column command.
std::string SqlserverGrammar::compile_add(Blueprint *blueprint, fluent_t &) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "alter table %s add %s",
           wrap_table(blueprint).c_str(),
           implode(", ", get_columns(blueprint)).c_str());
  return temp;
}

//Compile a primary key command.
std::string SqlserverGrammar::compile_primary(
    Blueprint *blueprint, fluent_t &command) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "alter table %s add constraint %s primary key (%s)",
           wrap_table(blueprint).c_str(),
           wrap(command["index"]).c_str(),
           columnize(command.columns).c_str());
  return temp;
}

//Compile a unique key command.
std::string SqlserverGrammar::compile_unique(
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

//Compile a plain index key command.
std::string SqlserverGrammar::compile_index(
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

//Compile a drop table (if exists) command.
std::string SqlserverGrammar::compile_drop_if_exists(
    Blueprint *blueprint, fluent_t &command) {
  UNUSED(command);
  char temp[1024]{0};
  std::string table = "'" + str_replace("'", "''", 
      get_table_prefix() + blueprint->get_table()) + "'";
  snprintf(temp,
           sizeof(temp) - 1,
           "if exists (select * from INFORMATION_SCHEMA.TABLES where"
           " TABLE_NAME = %s) drop table %s",
           table.c_str(),
           wrap_table(blueprint).c_str());
  return temp;
}

//Compile a drop column command.
std::string SqlserverGrammar::compile_drop_column(
    Blueprint *blueprint, fluent_t &command) {
  auto columns = wrap_array(command.columns);
  return "alter table " + 
         wrap_table(blueprint) + " drop column " + implode(", ", columns);
}

//Compile a drop primary key command.
std::string SqlserverGrammar::compile_drop_primary(
   Blueprint *blueprint, fluent_t &command) {
  auto index = wrap(command["index"]);
  return "alter table " + wrap_table(blueprint) + " drop constraint " + index;
}

//Compile a drop unique key command.
std::string SqlserverGrammar::compile_drop_unique(
   Blueprint *blueprint, fluent_t &command) {
  auto index = wrap(command["index"]);
  return "drop index " + index + " on " + wrap_table(blueprint);
}

//Get the SQL for a collation column modifier.
std::string SqlserverGrammar::modify_collate(
    Blueprint *, fluent_t &column) {
  if (!empty(column["collation"]))
    return " collate " + column["collation"].data;
  return "";
}

//Get the SQL for a default column modifier.
std::string SqlserverGrammar::modify_default(
    Blueprint *blueprint, fluent_t &column) {
  UNUSED(blueprint);
  if (!empty(column["default"]))
    return " default " + get_default_value(column["default"]);
  return "";
}

//Get the SQL for an auto-increment column modifier.
std::string SqlserverGrammar::modify_increment(
    Blueprint *, fluent_t &column) {
  if (in_array(column["type"].data, serials_) && 
      column["auto_increment"] == true) {
    return " identity primary key";
  }
  return "";
}
