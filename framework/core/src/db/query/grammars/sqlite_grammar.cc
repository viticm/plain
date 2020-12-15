#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/query/grammars/sqlite_grammar.h"

using namespace pf_db::query::grammars;
using namespace pf_basic::string;
using namespace pf_basic::type;
using namespace pf_support;

SqliteGrammar::SqliteGrammar() {

select_components_ = {
  "aggregate",
  "columns",
  "from",
  "joins",
  "wheres",
  "groups",
  "havings",
  "orders",
  "limit",
  "offset",
  "lock",
};

operators_ = {
  "=", "<", ">", "<=", ">=", "<>", "!=",
  "like", "not like", "between", "ilike",
  "&", "|", "<<", ">>",
};

}

SqliteGrammar::~SqliteGrammar() {

}

//Compile a select query into SQL.
std::string SqliteGrammar::compile_select(Builder &query) {
  auto sql = Grammar::compile_select(query);
  if (!query.unions_.empty())
    sql = "select * from (" + sql + ") " + compile_unions(query);
  return sql;
}

//Compile an insert statement into SQL.
std::string SqliteGrammar::compile_insert(
    Builder &query, std::vector<variable_set_t> &values) {
  if (values.empty()) return "";
  // Essentially we will force every insert to be treated as a batch insert which 
  // simply makes creating the SQL easier for us since we can utilize the same
  // basic routine regardless of an amount of records given to us to insert.
  auto table = wrap_table(query.from_);

  // If there is only one record being inserted, we will just use the usual query 
  // grammar insert builder because no special syntax is needed for the single 
  // row inserts in SQLite. However, if there are multiples, we'll continue.
  if (1 == values.size())
    return Grammar::compile_insert(query, values);

  auto _columns = array_keys(values[0]);
  auto names = columnize(_columns);

  // SQLite requires us to build the multi-row insert as a listing of select with
  // unions joining them together. So we'll build out this list of columns and
  // then join them all together with select unions to complete the queries.
  std::vector<std::string> columns;
  for (const std::string &column : _columns)
    columns.emplace_back("? as " + wrap(column));

  std::vector<std::string> rcolumns;
  auto columns_str = implode(", ", columns);
  for (size_t i = 0; i < values.size(); ++i)
    rcolumns.emplace_back(columns_str);
  return "insert into " + table + " (" + names + ") select " +
         implode(" union all select ", rcolumns);
}

//Compile a truncate table statement into SQL.
variable_set_t SqliteGrammar::compile_truncate(Builder &query) {
  return {
    {"delete from sqlite_sequence where name = ?", query.from_},
    {"delete from " + wrap_table(query.from_), {}},
  };
}

//Compile a single union statement.
std::string SqliteGrammar::compile_union(db_query_array_t &_union) {
  if (is_null(_union.query)) return "";
  std::string conjunction = !empty(_union["all"]) && (_union["all"] == true) ? 
                           " union all " : " union ";
  return conjunction + "select * from (" + _union.query->to_sql() + ")";
}

//Compile a date based where clause.
std::string SqliteGrammar::date_based_where(
    const std::string &type, Builder &query, db_query_array_t &where) {
  UNUSED(query);
  std::string value = where["value"].data;
  if (value.size() < 2) {
    for (size_t i = 0; i < 2 - value.size(); ++i)
      value = "0" + value;
  }
  value = parameter(value);
  return "strftime('" + type + "', " + wrap(where["column"]) + ") " + 
          where["operator"].data + " " + value;
}
