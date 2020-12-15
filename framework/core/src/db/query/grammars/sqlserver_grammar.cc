#include <algorithm>
#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/query/grammars/sqlserver_grammar.h"

using namespace pf_db::query::grammars;
using namespace pf_basic::string;
using namespace pf_basic::type;
using namespace pf_support;

SqlserverGrammar::SqlserverGrammar() {
  operators_ = {
    "=", "<", ">", "<=", ">=", "!<", "!>", "<>", "!=",
    "like", "not like", "between", "ilike",
    "&", "&=", "|", "|=", "^", "^=",
  };
}

SqlserverGrammar::~SqlserverGrammar() {

}

//Compile a select query into SQL.
std::string SqlserverGrammar::compile_select(Builder &query) {
  if (-1 == query.offset_)
    return Grammar::compile_select(query);

  // If an offset is present on the query, we will need to wrap the query in
  // a big "ANSI" offset syntax block. This is very nasty compared to the
  // other database systems but is necessary for implementing features.
  if (query.columns_.empty())
    query.columns_ = {"*"};
  variable_set_t components = compile_components(query);
  return compile_ansi_offset(query, components);
}

//Compile a truncate table statement into SQL.
variable_set_t SqlserverGrammar::compile_truncate(Builder &query) {
  return {{"truncate table " + wrap_table(query.from_), ""}};
}

//Compile an exists statement into SQL.
std::string SqlserverGrammar::compile_exists(Builder &query) {
  auto columns = query.columns_;
  query.columns_ = {};
  auto sql = compile_select(query.select_raw("1 [exists]").limit(1));
  query.columns_ = columns;
  return sql;
}

//Compile an update statement into SQL.
std::string SqlserverGrammar::compile_update(
    Builder &query, variable_set_t &values) {
  auto temp = parse_update_table(query.from_);
  auto table = temp[0];
  auto alias = temp[1];
  // Each one of the columns in the update statements needs to be wrapped in the
  // keyword identifiers, also a place-holder needs to be created for each of
  // the values in the list of bindings so we can make the sets statements.
  std::vector<std::string> _columns;
  for (auto it = values.begin(); it != values.end(); ++it)
    _columns.emplace_back(wrap(it->first) + " = " + parameter(it->second));

  auto columns = implode(", ", _columns);

  // If the query has any "join" clauses, we will setup the joins on the builder 
  // and compile them so we can attach them to this update, as update queries 
  // can get join statements to attach to other tables when they're needed.
  std::string joins{""};

  if (!query.joins_.empty())
    joins = " " + compile_joins(query, query.joins_);

  // Of course, update queries may also be constrained by where clauses so we'll
  // need to compile the where clauses and attach it to the query so only the
  // intended records are updated by the SQL statements we generate to run.
  
  auto where = compile_wheres(query);

  std::string sql = "update "+ table + joins + " set " + columns + " " + where;

  if (joins != "")
    return trim(
        sql = "update " + alias + " set " + columns + " from " + 
        table + joins + " " + where);
  return trim(sql);
}

//Prepare the bindings for an update statement.
variable_array_t SqlserverGrammar::prepare_bindings_forupdate(
    db_query_bindings_t &bindings, const variable_array_t &values) {
  // Update statements with joins in SQL Servers utilize an unique syntax. We need to
  // take all of the bindings and put them on the end of this array since they are
  // added to the end of the "where" clause statements as typical where clauses.
  variable_array_t r;
  variable_array_t bindings_without_join;
  for (const std::string &key : DB_BINDING_KEYS) {
    if (key != "join") {
      for (const variable_t &value : bindings[key])
        bindings_without_join.emplace_back(value);
    }
  }
  for (const variable_t &value : values)
    r.emplace_back(value);
  for (const variable_t &value : bindings["join"])
    r.emplace_back(value);
  for (const variable_t &value: bindings_without_join)
    r.emplace_back(value);
  return r;
}

//Compile a delete statement into SQL.
std::string SqlserverGrammar::compile_delete(Builder &query) {
  auto table = wrap_table(query.from_);

  auto where = !query.wheres_.empty() ? compile_wheres(query) : "";
  
  std::string temp = "delete from " + table + " " + where;

  return !query.joins_.empty() 
           ? compile_delete_with_joins(query, table, where)
           : trim(temp);
}

//Compile the "select *" portion of the query.
std::string SqlserverGrammar::compile_columns(
    Builder &query, const variable_array_t &columns) {
  if (!query.aggregate_.empty()) return "";
  
  std::string select = query.distinct_ ? "select distinct " : "select ";

  // If there is a limit on the query, but not an offset, we will add the top 
  // clause to the query, which serves as a "limit" type clause within the 
  // SQL Server system similar to the limit keywords available in MySQL.
  if (query.limit_ > 0 && query.offset_ <= 0)
    select += "top " + std::to_string(query.limit_) + " ";

  return select + columnize(columns);
}

//Compile the "from" portion of the query.
std::string SqlserverGrammar::compile_from(
    Builder &query, const std::string &table) {
  auto from = Grammar::compile_from(query, table);

  if (kVariableTypeString == query.lock_.type)
    return from + " " + query.lock_.data;

  if (!empty(query.lock_))
    return from + " with(rowlock," + 
           (query.lock_.get<bool>() ? "updlock," : "") + "holdlock)";
  return from;
}

//Compile a "where date" clause.
std::string SqlserverGrammar::where_date(
    Builder &query, db_query_array_t &where) {
  UNUSED(query);
  auto value = parameter(where["value"]);
  return "cast(" + wrap(where["column"]) +" as date) " + 
         where["operator"].data + " " + value;
}

//Create a full ANSI offset clause for the query.
std::string SqlserverGrammar::compile_ansi_offset(
    Builder &query, variable_set_t &components) {
  // An ORDER BY clause is required to make this offset query work, so if one does 
  // not exist we'll just create a dummy clause to trick the database and so it
  // does not complain about the queries for not having an "order by" clause.
  if (components["orders"] == "")
    components["orders"] = "order by (select 0)";

  // We need to add the row number to the query so we can compare it to the offset
  // and limit values given for the statements. So we will add an expression to
  // the "select" that will give back the row numbers on each of the records.
  components["columns"] += compile_over(components["orders"].data);

  components.erase(components.find("orders"));

  // Next we need to calculate the constraints that should be placed on the query
  // and limit values given for the statements. So we will add an expression to
  // set we will just handle the offset only since that is all that matters.
  auto sql = concatenate(components);

  return compile_table_expression(sql, query);
}

//Compile the over statement for a table expression.
std::string SqlserverGrammar::compile_over(const std::string &orderings) {
  return  ", row_number() over (" + orderings + ") as row_num";
}

//Compile a common table expression for a query.
std::string SqlserverGrammar::compile_table_expression(
    const std::string &sql, Builder &query) {
  auto constraint = compile_row_constraint(query);

  return 
    "select * from (" + sql + ") as temp_table where row_num " + constraint;
}

//Compile the limit / offset row constraint for a query.
std::string SqlserverGrammar::compile_row_constraint(Builder &query) {
  auto start = query.offset_ + 1;
  
  if (query.limit_ > 0) {
    auto finish = query.offset_ + query.limit_;
    return "between " + std::to_string(start) + " and " + std::to_string(finish);
  }
  return ">= " + std::to_string(start);
}

//Compile a delete query that uses joins.
std::string SqlserverGrammar::compile_delete_with_joins(
    Builder &query, const std::string &table, const std::string &where) {
  std::string joins = " " + compile_joins(query, query.joins_);

  std::string alias{table};
  std::string temp{table};
  std::transform(
      temp.begin(), temp.end(), temp.begin(), (int (*)(int))std::tolower);
  if (temp.find(" as ") != std::string::npos) 
    alias = explode(" as ", temp)[0].data;

  std::string sql = "delete " + alias + " from " + table +  joins + " " + where;
  return trim(sql);
}

//Get the table and alias for the given table.
std::vector<std::string> SqlserverGrammar::parse_update_table(
    const std::string &table) {
  std::string _table, alias;
  _table = alias = wrap_table(table);
  std::string temp{table};
  std::transform(
      temp.begin(), temp.end(), temp.begin(), (int (*)(int))std::tolower);
  if (temp.find("] as [") != std::string::npos) 
    alias = "[" + explode("] as [", temp)[0].data;

  return {_table, alias};
}

//Wrap a single string in keyword identifiers.
std::string SqlserverGrammar::wrap_value(const variable_t &value) {
  return value == "*" ? value.data
          : "[" + str_replace("]", "]]", value.data) + "]";
}

//Wrap a table in keyword identifiers.
std::string SqlserverGrammar::wrap_table_value_function(
    const std::string &table) {
  std::string r{table};
  auto size = table.size();
  if (table.size() >= 5) { //Need preg math: ^(.+?)(\(.*?\))]$ like: [a](b)
    auto find_pos = table.find("(");
    if (find_pos != std::string::npos && 
        ']' == table[size - 1] && ')' == table[size - 2]) {
      r = table.substr(0, find_pos) + "]" + /* [a] */
            table.substr(find_pos, table.find(")") - find_pos + 1); /* (b) */
    }
  }
  return r;
}
