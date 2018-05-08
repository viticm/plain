#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/connection_interface.h"
#include "pf/db/schema/blueprint.h"
#include "pf/db/schema/grammars/mysql_grammar.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_db::schema::grammars;

MysqlGrammar::MysqlGrammar() {
  modifiers_ = {
    "virtual_as", "stored_as", "unsigned", "charset", "collate", "nullable",
    "default", "increment", "comment", "after", "first",
  };

  serials_ = {
    "big_integer", "integer", "medium_integer", "small_integer", "tiny_integer",
  };
}

//Compile a create table command.
std::string MysqlGrammar::compile_create(Blueprint *blueprint, 
                                         fluent_t &command, 
                                         ConnectionInterface *connection) {
  std::string sql = compile_create_table(blueprint, command, connection);

  // Once we have the primary SQL, we can add the encoding option to the SQL for 
  // the table.  Then, we can check if a storage engine has been supplied for
  // the table. If so, we will add the engine declaration to the SQL query.
  sql = compile_create_encoding(sql, connection, blueprint);

  // Finally, we will append the engine configuration onto this SQL statement as
  // the final thing we do before returning this finished SQL. Once this gets 
  // added the query will be ready to execute against the real connections.
  return compile_create_engine(sql, connection, blueprint);
}

//Create the main create table clause.
std::string MysqlGrammar::compile_create_table(Blueprint *blueprint, 
                                               fluent_t &command, 
                                               ConnectionInterface *connection) {

  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "%s table %s (%s)",
           blueprint->temporary_ ? "create temporary" : "create",
           wrap_table(blueprint).c_str(),
           implode(", ", get_columns(blueprint)).c_str());
  return temp;
}

//Append the character set specifications to a command.
std::string MysqlGrammar::compile_create_encoding(
    const std::string &sql, 
    ConnectionInterface *connection,
    Blueprint *blueprint) {
  std::string r{sql};
  // First we will set the character set if one has been set on either the create 
  // blueprint itself or on the root configuration for the connection that the
  // table is being created on. We will add these to the create table query.
  if (!blueprint->charset_.empty()) {
    r += " default character set " + blueprint->charset_;
  } else {
    auto charset = connection->get_config("charset").data;
    if (!charset.empty())
      r += " default character set " + charset;
  }

  // Next we will add the collation to the create table statement if one has been 
  // added to either this create table blueprint or the configuration for this
  // connection that the query is targeting. We'll add it to this SQL query.
  if (!blueprint->collation_.empty()) {
    r += " collate " + blueprint->collation_;
  } else {
    auto collation = connection->get_config("collation").data;
    if (!collation.empty())
      r += " collate " + collation;
  }
  return r;
}

//Append the engine specifications to a command.
std::string MysqlGrammar::compile_create_engine(
    const std::string &sql, 
    ConnectionInterface *connection,
    Blueprint *blueprint) {
  std::string r{sql};
  if (!blueprint->engine_.empty()) {
    r += " engine = " + blueprint->engine_;
  } else {
    auto engine = connection->get_config("engine").data;
    if (!engine.empty())
      r += " engine = " + engine;
  }
  return r;
}

//Compile an add column command.
std::string MysqlGrammar::compile_add(Blueprint *blueprint, fluent_t &command) {
  auto columns = prefix_array("add", get_columns(blueprint));
  return "alter table " + wrap_table(blueprint) + " " + implode(", ", columns);
};

//Compile a drop column command.
std::string MysqlGrammar::compile_drop_column(
    Blueprint *blueprint, fluent_t &command) {
  auto columns = prefix_array("drop", wrap_array(command.columns));
  return "alter table " + wrap_table(blueprint) + " " + implode(", ", columns);
}

//Create the column definition for a double type.
std::string MysqlGrammar::type_double(fluent_t &column) const {
  if (!empty(column["total"]) && !empty(column["places"]))
    return "double(" + column["total"].data + ", " + column["places"].data + ")";
  return "double";
}

//Create the column definition for an enum type.
std::string MysqlGrammar::type_enum(fluent_t &column) const {
  variable_array_t allowed;
  for (auto value : column.allowed)
    allowed.emplace_back(value);
  return "enum('" + implode(", ", allowed) + "')";
}

//Create the column definition for a timestamp type.
std::string MysqlGrammar::type_timestamp(fluent_t &column) const {
  if (!empty(column["use_current"]))
    return "timestamp default CURRENT_TIMESTAMP";
  return "timestamp";
}

//Get the SQL for a generated virtual column modifier.
std::string MysqlGrammar::modify_virtual_as(Blueprint *, fluent_t &column) {
  if (!empty(column["virtual_as"]))
    return " as (" + column["virtual_as"].data + ")";
  return "";
}

//Get the SQL for a generated stored column modifier.
std::string MysqlGrammar::modify_stored_as(Blueprint *, fluent_t &column) {
  if (!empty(column["stored_as"]))
    return " as (" + column["stored_as"].data + ") stored";
  return "";
}

//Get the SQL for an unsigned column modifier.
std::string MysqlGrammar::modify_unsigned(Blueprint *, fluent_t &column) {
  if (!empty(column["unsigned"]))
    return " unsigned";
  return "";
}

//Get the SQL for a character set column modifier.
std::string MysqlGrammar::modify_charset(Blueprint *, fluent_t &column) {
  if (!empty(column["charset"]))
    return " character set " + column["charset"].data;
  return "";
}

//Get the SQL for a collation column modifier.
std::string MysqlGrammar::modify_collate(Blueprint *, fluent_t &column) {
  if (!empty(column["collation"]))
    return " collate " + column["collation"].data;
  return "";
}

//Get the SQL for a default column modifier.
std::string MysqlGrammar::modify_nullable(Blueprint *, fluent_t &column) {
  if (empty(column["virtual_as"]) && empty(column["stored_as"]))
    return column["nullable"] == true ? " null" : " not null";
  return "";
}

//Get the SQL for a default column modifier.
std::string MysqlGrammar::modify_default(Blueprint *, fluent_t &column) {
  if (!empty(column["default"]))
    return " default " + get_default_value(column["default"]);
  return "";
}


//Get the SQL for an auto-increment column modifier.
std::string MysqlGrammar::modify_increment(Blueprint *, fluent_t &column) {
  if (in_array(column["type"].data, serials_) && column["auto_increment"] == true)
    return " auto_increment primary key";
  return "";
}

//Get the SQL for a "first" column modifier.
std::string MysqlGrammar::modify_first(Blueprint *, fluent_t &column) {
  if (!empty(column["first"]))
    return " first";
  return "";
}

//Get the SQL for an "after" column modifier.
std::string MysqlGrammar::modify_after(Blueprint *, fluent_t &column) {
  if (!empty(column["after"]))
    return " after " + wrap(column["after"]);
  return "";
}

//Get the SQL for a "comment" column modifier.
std::string MysqlGrammar::modify_comment(Blueprint *, fluent_t &column) {
  if (!empty(column["comment"]))
    return " comment '" + addslashes(column["comment"]) + "'";
  return "";
}

//Wrap a single string in keyword identifiers.
std::string MysqlGrammar::wrap_value(const variable_t &value) {
  if (value != "*")
    return "`" + str_replace("`", "``", value.data) + "`";
  return value.data;
}
