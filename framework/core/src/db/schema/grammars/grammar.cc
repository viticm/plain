#include "pf/support/helpers.h"
#include "pf/db/schema/blueprint.h"
#include "pf/db/schema/grammars/rename_column.h"
#include "pf/db/schema/grammars/change_column.h"
#include "pf/db/schema/grammars/grammar.h"

using namespace pf_support;
using namespace pf_basic::type;
using namespace pf_db::schema::grammars;

#define compile_call(name) \
  [this](Blueprint *blueprint, fluent_t &command) { \
     return compile_##name(blueprint, command); \
  }

#define safe_call_compile(name, blueprint, command) \
  (compile_calls_.find(name) != compile_calls_.end() ? \
   compile_calls_[name](blueprint, command) : "")

#define type_call(name) \
  [this](fluent_t &column) { \
     return type_##name(column); \
  }

#define safe_call_type(name, column) \
  (type_calls_.find(name) != type_calls_.end() ? \
   type_calls_[name](column) : "")

#define modify_call(name) \
  [this](Blueprint *blueprint, fluent_t &column) { \
     return modify_##name(blueprint, column); \
  }

#define safe_call_modify(name, blueprint, column) \
  (modify_calls_.find(name) != modify_calls_.end() ? \
   modify_calls_[name](blueprint, column) : "")

Grammar::Grammar() {
  compile_calls_ = {
    {"add", compile_call(add) },
    {"foreign", compile_call(foreign) },
    {"primary", compile_call(primary) },
    {"unique", compile_call(unique) },
    {"index", compile_call(index) },
    {"drop", compile_call(drop) },
    {"drop_if_exists", compile_call(drop_if_exists) },
    {"drop_column", compile_call(drop_column) },
    {"drop_primary", compile_call(drop_primary) },
    {"drop_unique", compile_call(drop_unique) },
    {"drop_index", compile_call(drop_index) },
    {"drop_foreign", compile_call(drop_foreign) },
  };
  type_calls_ = {
    {"char", type_call(char) },
    {"string", type_call(string) },
    {"text", type_call(text) },
    {"medium_text", type_call(medium_text) },
    {"long_text", type_call(long_text) },
    {"big_integer", type_call(big_integer) },
    {"integer", type_call(integer) },
    {"medium_integer", type_call(medium_integer) },
    {"tiny_integer", type_call(tiny_integer) },
    {"small_integer", type_call(small_integer) },
    {"float", type_call(float) },
    {"double", type_call(double) },
    {"decimal", type_call(decimal) },
    {"boolean", type_call(boolean) },
    {"enum", type_call(enum) },
    {"json", type_call(json) },
    {"jsonb", type_call(jsonb) },
    {"date", type_call(date) },
    {"date_time", type_call(date_time) },
    {"date_time_tz", type_call(date_time_tz) },
    {"timestamp", type_call(timestamp) },
    {"binary", type_call(binary) },
    {"uuid", type_call(uuid) },
    {"ip_address", type_call(ip_address) },
    {"mac_address", type_call(mac_address) },
  };

  modify_calls_ = {
    {"virtual_as", modify_call(virtual_as) },
    {"stored_as", modify_call(stored_as) },
    {"unsigned", modify_call(unsigned) },
    {"charset", modify_call(charset) },
    {"collate", modify_call(collate) },
    {"default", modify_call(default) },
    {"increment", modify_call(increment) },
    {"first", modify_call(first) },
    {"after", modify_call(after) },
    {"comment", modify_call(comment) },
  };
}

//The function call compile.
std::vector<std::string> Grammar::call_compile(
    Blueprint *blueprint, 
    fluent_t &command, 
    ConnectionInterface *connection, 
    const std::string &method) {
  std::vector<std::string> r;
  if ("rename_column" == method) {
    std::vector<std::string> rs = 
      compile_rename_column(blueprint, command, connection);
    for (const std::string &item : rs)
      r.emplace_back(item);
  } else if ("change" == method) {
    std::vector<std::string> rs = 
      compile_change(blueprint, command, connection);
    for (const std::string &item : rs)
      r.emplace_back(item);
  } else if ("create" == method) {
    std::string rs = compile_create(blueprint, command, connection);
    if (rs != "") r.emplace_back(rs);
  } else {
    std::string rs = safe_call_compile(method, blueprint, command);
    if (rs != "") r.emplace_back(rs);
  }
  return r;
}

//The function call type.
std::string Grammar::call_type(fluent_t &column, const std::string &method) {
  return safe_call_type(method, column);
}

//The function call modify.
std::string Grammar::call_modify(
    Blueprint *blueprint, fluent_t &column, const std::string &method) {
  return safe_call_modify(method, blueprint, column);
}

//Compile a rename column command.
std::vector<std::string> Grammar::compile_rename_column(
    Blueprint *blueprint, fluent_t &command, ConnectionInterface *connection) {
  return RenameColumn::compile(this, blueprint, command, connection);
}

//Compile a change column command into a series of SQL statements.
std::vector<std::string> Grammar::compile_change(
 Blueprint *blueprint, fluent_t &command, ConnectionInterface *connection) {
  return ChangeColumn::compile(this, blueprint, command, connection);
}

//Compile a foreign key command.
std::string Grammar::compile_foreign(Blueprint *blueprint, fluent_t &command) {
  char temp[1024]{0,};
  std::string sql{""};
  
  // We need to prepare several of the elements of the foreign key definition
  // before we can create the SQL, such as wrapping the tables and convert
  // an array of columns to comma-delimited strings for the SQL queries.
  snprintf(temp, 
           sizeof(temp) - 1, 
           "alter table %s add constraint %s ",
           wrap_table(blueprint).c_str(),
           wrap(command["index"]).c_str());
  sql += temp;
  memset(temp, 0, sizeof(temp) - 1);

  // Once we have the initial portion of the SQL statement we will add on the 
  // key name, table name, and referenced columns. These will complete the 
  // main portion of the SQL statement and this SQL will almost be done.
  snprintf(temp, 
           sizeof(temp) - 1, 
           "foreign key (%s) references %s (%s)",
           columnize(command.columns).c_str(),
           wrap_table(command["on"]).c_str(),
           columnize(command.references).c_str());
  sql += temp;
  memset(temp, 0, sizeof(temp) - 1);

  // Once we have the basic foreign key creation statement constructed we can
  // build out the syntax for what should happen on an update or delete of
  // the affected columns, which will get something like "cascade", etc.
  if (!empty(command["on_delete"])) {
    sql += " on delete " + command["on_delete"].data;
  }

  if (!empty(command["on_update"])) {
    sql += " on update " + command["on_update"].data;
  }
  return sql;
}

//Add a prefix to an array of values.
Grammar::variable_array_t Grammar::prefix_array(
    const std::string &prefix, const variable_array_t &values) {
  variable_array_t r;
  for (const variable_t &value : values)
    r.emplace_back(prefix + " " + value.data);
  return r;
}

//Add a prefix to an array of values.
Grammar::variable_array_t Grammar::prefix_array(
    const std::string &prefix, const std::vector<std::string> &values) {
  variable_array_t r;
  for (auto &value : values)
    r.emplace_back(prefix + " " + value);
  return r;
}

//Wrap a table in keyword identifiers.
std::string Grammar::wrap_table(Blueprint *blueprint) {
  return pf_db::Grammar::wrap_table(blueprint->get_table());
}

//Wrap a value in keyword identifiers.
std::string Grammar::wrap(const variable_t &value, bool) {
  return pf_db::Grammar::wrap_table(value);
}

//Create an empty Doctrine DBAL TableDiff from the Blueprint.
bool Grammar::get_doctrine_table_diff(Blueprint *blueprint, void *schema) {
  return false;
}

//Compile the blueprint's column definitions.
std::vector<std::string> Grammar::get_columns(Blueprint *blueprint) {
  std::vector<std::string> columns;
  auto add_columns = blueprint->get_added_columns();
  for (auto &column : add_columns) {
    // Each of the column types have their own compiler functions which are tasked
    // with turning the column definition into its SQL format for this platform
    // used by the connection. The column's modifiers are compiled and added.
    std::string sql = wrap(column) + " " + get_type(column);
    columns.emplace_back(add_modifiers(sql, blueprint, column));
  }
  return columns;
}

//Get the SQL for the column data type.
std::string Grammar::get_type(fluent_t &column) {
  return call_type(column, column["type"].data);
}

//Add the column modifiers to the definition.
std::string Grammar::add_modifiers(const std::string &sql, 
                                   Blueprint *blueprint, 
                                   fluent_t &column) {
  std::string r{sql};
  for (const std::string &modifier : modifiers_) {
    auto temp = call_modify(blueprint, column, modifier);
    if (temp != "") r += temp;
  }
  return r;
}

//Get the primary key command if it exists on the blueprint.
Grammar::fluent_t Grammar::get_command_by_name(
    Blueprint *blueprint, const std::string &name) {
  auto commands = get_commands_by_name(blueprint, name);
  fluent_t r;
  if (commands.size() > 0) r = commands[0];
  return r;
}

//Get all of the commands with a given name.
std::vector<Grammar::fluent_t> Grammar::get_commands_by_name(
    Blueprint *blueprint, const std::string &name) {
  return array_filter<fluent_t>(
      blueprint->get_commands(), [&name](const fluent_t &val){
    auto it = val.items.find("name");
    return it == val.items.end() || it->second == name;
  });
}

//Format a value so that it can be used in "default" clauses.
std::string Grammar::get_default_value(const variable_t &value) {
  if (DB_EXPRESSION_TYPE == value.type) {
    return value.data;
  } else if (kVariableTypeBool == value.type) {
    char temp[128]{0};
    snprintf(temp, sizeof(temp) - 1, "'%s'", value == true ? "1" : "0");
    return temp;
  } else {
    return "'" + value.data + "'";
  }
}
