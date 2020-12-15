#include <algorithm>
#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/connection_interface.h"
#include "pf/db/schema/grammars/grammar.h"
#include "pf/db/schema/builder.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_db::schema;

//Reset values
void Blueprint::clear() {
  engine_ = "";
  charset_ = "";
  collation_ = "";
  temporary_ = false;
  table_ = "";
  columns_.clear();
  commands_.clear();
}

//Create a new char column on the table.
Blueprint::fluent_t &Blueprint::_char(
    const std::string &column, int32_t length) {
  int32_t rlength = -1 == length ? Builder::default_string_length_ : length;
  variable_set_t params = {{"length", rlength}, };
  return add_column("char", column, params);     
}

//Create a new string column on the table.
Blueprint::fluent_t &Blueprint::string(
    const std::string &column, int32_t length) {
  int32_t rlength = -1 == length ? Builder::default_string_length_ : length;
  variable_set_t params = {{"length", rlength}, };
  return add_column("string", column, params);
}

//Create a default index name for the table.
std::string Blueprint::create_index_name(
    const std::string &type, 
    const std::vector<std::string> &columns) {
  std::string _index = table_ + "_" + implode("_", columns) + "_" + type;
  std::transform(
      _index.begin(), _index.end(), _index.begin(), (int (*)(int))std::tolower);
  return str_replaces({"-", "."}, "_", _index);
}

//Get the columns on the blueprint that should be added.
std::vector<Blueprint::fluent_t> Blueprint::get_added_columns() {
  return array_filter<fluent_t>(columns_, [](const fluent_t &val){
    auto it = val.items.find("change");
    return it == val.items.end() || !it->second.get<bool>();
  });
}

//Get the columns on the blueprint that should be changed.
std::vector<Blueprint::fluent_t> Blueprint::get_changed_columns() {
  return array_filter<fluent_t>(columns_, [](const fluent_t &val){
    auto it = val.items.find("change");
    return it != val.items.end() && it->second.get<bool>();
  });
}

//Add the commands that are implied by the blueprint's state.
void Blueprint::add_implied_commands() {
  if (get_added_columns().size() > 0 && !creating()) {
    std::vector<fluent_t> temp;
    temp.emplace_back(create_command("add"));
    for (auto &item : commands_)
      temp.emplace_back(item);
    commands_ = temp;
  }
  if (get_changed_columns().size() > 0 && !creating()) {
    std::vector<fluent_t> temp;
    temp.emplace_back(create_command("change"));
    for (auto &item : commands_)
      temp.emplace_back(item);
    commands_ = temp;
  }
  add_fluent_indexes();
}

//Add the index commands fluently specified on columns.
void Blueprint::add_fluent_indexes() {
  auto call = [this](const std::string &name, 
                 const std::string &column, 
                 const std::string &index) {
    if ("primary" == name) {
      this->primary({column}, index);
    } else if ("unique" == name) {
      this->unique({column}, index);
    } else if ("index" == name) {
      this->index({column}, index);
    }
  };
  for (auto &column : columns_) {
    for (const std::string &_index : {"primary", "unique", "index"}) {
      // If the index has been specified on the given column, but is simply equal
      // to "true" (boolean), no name has been specified for this index so the
      // index method can be called without a name and it will generate one.
      if (column.items.find(_index) != column.items.end() && 
          column[_index].get<bool>()) {
        call(_index, column["name"], "");
        break;
      } else if (column.items.find(_index) != column.items.end()) {
        call(_index, column["name"], column[_index].c_str());
        break;
      }
    }
  }
}

//Determine if the blueprint has a create command.
bool Blueprint::creating() {
  bool result{false};
  for (auto &command : commands_) {
    if (command["name"] == "create") {
      result = true;
      break;
    }
  }
  return result;
}

//Get the raw SQL statements for the blueprint. 
std::vector<std::string> Blueprint::to_sql(
    ConnectionInterface *connection, grammars::Grammar *grammar) {
  add_implied_commands();
  std::vector<std::string> statements;

  // Each type of command has a corresponding compiler function on the schema
  // grammar which is used to build the necessary SQL statements to build
  // the blueprint element, so we'll just call that compilers function.
  for (fluent_t &command : commands_) {
    //std::cout << "command[\"name\"]: " << command["name"].data << std::endl;
    auto sqls = grammar->call_compile(this, command, connection, command["name"]);
    for (const std::string &sql : sqls) {
      if (sql != "") statements.emplace_back(sql);
    }
  }
  return statements;
}

//Execute the blueprint against the database.
void Blueprint::build(ConnectionInterface *connection, 
                      grammars::Grammar *grammar) {
  auto statements = to_sql(connection, grammar);
  for (const std::string &statement : statements)
    connection->statement(statement);
}
