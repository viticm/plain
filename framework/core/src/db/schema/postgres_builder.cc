#include "pf/support/helpers.h"
#include "pf/db/connection_interface.h"
#include "pf/db/schema/grammars/grammar.h"
#include "pf/db/schema/postgres_builder.h"

using namespace pf_support;
using namespace pf_db::schema;

//Determine if the given table exists.
bool PostgresBuilder::has_table(const std::string &a_table) {
  auto schema = connection_->get_config("schema");
  if (empty(schema)) schema = "public";
  std::string _table = connection_->get_table_prefix() + a_table;
  return connection_->select(
    grammar_->compile_table_exists(), {schema.c_str(), _table}
  ).size() > 0;
}
