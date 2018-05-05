#include "pf/db/connection_interface.h"
#include "pf/db/schema/grammars/grammar.h"
#include "pf/db/schema/mysql_builder.h"

using namespace pf_db::schema;

//Determine if the given table exists.
bool MysqlBuilder::has_table(const std::string &table) {
  std::string _table = connection_->get_table_prefix() + table;
  return connection_->select(
    grammar_->compile_table_exists(), {connection_->get_database_name(), _table}
  ).size() > 0;
}

//Get the column listing for a given table.
std::vector<std::string> MysqlBuilder::get_column_listing(
    const std::string &table) {
  std::string _table = connection_->get_table_prefix() + table;
  auto results =  connection_->select(
    grammar_->compile_column_listing(), {connection_->get_database_name(), _table}
  );
  std::vector<std::string> r;
  for (auto &key : results.keys)
    r.emplace_back(key);
  return r;
}
