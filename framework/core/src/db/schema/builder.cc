#include <algorithm>
#include "pf/support/helpers.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/grammar.h"
#include "pf/db/schema/builder.h"

using namespace pf_support;
using namespace pf_db::schema;

//Determine if the given table exists.
bool Builder::has_table(const std::string &table) {
  std::string _table = connection_->get_table_prefix() + table;
  return connection_->select(grammar_->compile_table_exists(), 
         {_table}).size() > 0;
}

//Determine if the given table has a given column.
bool Builder::has_column(const std::string &table, const std::string &column) {
  auto columns = get_column_listing(table);
  bool result = false;
  for (const std::string &item : columns) {
    std::string temp{item};
    std::transform(
      temp.begin(), temp.end(), temp.begin(), (int (*)(int))std::tolower);
    if (temp == column) {
      result = true;
      break;
    }
  }
  return result;
}

//Determine if the given table has given columns.
bool Builder::has_columns(const std::string &table, 
                          const std::vector<std::string> &columns) {
  auto _columns = get_column_listing(table);
  bool result = true;
  for (const std::string &item : _columns) {
    std::string temp{item};
    std::transform(
      temp.begin(), temp.end(), temp.begin(), (int (*)(int))std::tolower);
    if (!in_array(temp, columns)) {
      result = false;
      break;
    }
  }
  return result;
}

//Get the data type for the given column name.
std::string Builder::get_column_type(
    const std::string &table, const std::string &column) {
  
}

//Get the column listing for a given table.
std::vector<std::string> Builder::get_column_listing(const std::string &table) {
  std::string _table = connection_->get_table_prefix() + table;
  auto results = connection_->select(grammar_->compile_column_listing(_table));
  std::vector<std::string> r;
  for (auto &key : results.keys)
    r.emplace_back(key);
  return r;
}

//Create a new table on the schema.
void Builder::create(const std::string &table, Blueprint::closure_t callback) {
  std::unique_ptr<Blueprint> blueprint(create_blueprint(table));
  blueprint->tap([&callback](Blueprint *o){
    o->create();
    callback(o);
  });
  build(blueprint);
}
