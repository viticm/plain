#include "pf/support/helpers.h"
#include "pf/db/schema/blueprint.h"
#include "pf/db/schema/grammars/postgres_grammar.h"

using namespace pf_support;
using namespace pf_db::schema::grammars;

PostgresGrammar::PostgresGrammar() {
  modifiers_ = {
    "increment", "nullable", "default",
  };
  serials_ = {
    "big_integer", "integer", "medium_integer", "small_integer", "tiny_integer",
  };
}

//Compile a create table command.
std::string PostgresGrammar::compile_create(Blueprint *blueprint, 
                                            fluent_t &, 
                                            ConnectionInterface *) {
  char temp[1024];
  snprintf(temp,
           sizeof(temp) - 1,
           "%s table %s (%s)",
           blueprint->temporary_ ? "create temporary" : "create",
           wrap_table(blueprint).c_str(),
           implode(", ", get_columns(blueprint)).c_str());
  return temp;
}

//Compile an add column command.
std::string PostgresGrammar::compile_add(Blueprint *blueprint, fluent_t &) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "alter table %s %s",
           wrap_table(blueprint).c_str(),
           implode(
             ", ", prefix_array("add column", get_columns(blueprint))).c_str());
  return temp;
}

//Compile a unique key command.
std::string PostgresGrammar::compile_unique(
    Blueprint *blueprint, fluent_t &command) {
  char temp[1024]{0};
  snprintf(temp,
           sizeof(temp) - 1,
           "alter table %s add constraint %s unique (%s)",
           wrap_table(blueprint).c_str(),
           wrap(command["index"]).c_str(),
           columnize(command.columns).c_str());
  return temp;
}

//Compile a plain index key command.
std::string PostgresGrammar::compile_index(
    Blueprint *blueprint, fluent_t &command) {
  char temp[1024]{0};
  std::string algorithm = 
    empty(command["algorithm"]) ? "" : " using " + command["algorithm"].data;
  snprintf(temp,
           sizeof(temp) - 1,
           "create index %s on %s%s (%s)",
           wrap(command["index"]).c_str(),
           wrap_table(blueprint).c_str(),
           algorithm.c_str(),
           columnize(command.columns).c_str());
  return temp;
}

//Compile a drop column command.
std::string PostgresGrammar::compile_drop_column(
    Blueprint *blueprint, fluent_t &command) {
  auto columns = prefix_array("drop column", wrap_array(command.columns));
  return "alter table " + 
          wrap_table(blueprint) + " " + implode(", ", columns);
};

//Create the column definition for an enum type.
std::string PostgresGrammar::type_enum(fluent_t &column) const {
  std::vector<std::string> allowed;
  for (auto value : column.allowed)
    allowed.emplace_back("'" + value + "'");
  return "varchar(255) check (\"" + 
         column["name"].data + "\" in (" + implode(", ", allowed) + "))";
}

//Get the SQL for a default column modifier.
std::string PostgresGrammar::modify_default(Blueprint *, fluent_t &column) {
  if (!empty(column["default"]))
    return " default " + get_default_value(column["default"]);
  return "";
}

//Get the SQL for an auto-increment column modifier.
std::string PostgresGrammar::modify_increment(Blueprint *, fluent_t &column) {
  if (in_array(column["type"].data, serials_) && 
      column["auto_increment"] == true) {
    return " primary key";
  }
  return "";
}

//Compile a drop primary key command.
std::string PostgresGrammar::compile_drop_primary(
    Blueprint *blueprint, fluent_t &) {
  std::string index = wrap(blueprint->get_table() + "_pkey");
  return "alter table " + wrap_table(blueprint) + " drop constraint " + index;
}
