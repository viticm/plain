#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/query/grammars/mysql_grammar.h"

using namespace pf_db::query::grammars;
using namespace pf_basic::string;
using namespace pf_basic::type;
using namespace pf_support;

//The construct function.
MysqlGrammar::MysqlGrammar() {
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
}

//Compile a select query into SQL.
std::string MysqlGrammar::compile_select(Builder &query) {
  auto sql = Grammar::compile_select(query);
  if (!query.unions_.empty())
    sql = "(" + sql + ")" + compile_unions(query);
  return sql;
}

//Compile the random statement into SQL.
std::string MysqlGrammar::compile_random(const std::string &seed) {
  std::string r = "RAND(" + seed + ")";
  return r;
}

//Compile an update statement into SQL.
std::string MysqlGrammar::compile_update(
    Builder &query, variable_set_t &values) {
  auto table = wrap_table(query.from_);

  // Each one of the columns in the update statements needs to be wrapped in the 
  // keyword identifiers, also a place-holder needs to be created for each of 
  // the values in the list of bindings so we can make the sets statements.
  auto columns = compile_update_columns(values);

  // If the query has any "join" clauses, we will setup the joins on the builder
  // and compile them so we can attach them to this update, as update queries 
  // can get join statements to attach to other tables when they're needed.
  std::string joins{""};

  if (!query.joins_.empty())
    joins = " " + compile_joins(query, query.joins_);

  // Of course, update queries may also be constrained by where clauses so we'll
  // need to compile the where clauses and attach it to the query so only the 
  // intended records are updated by the SQL statements we generate to run.
  auto wheres = compile_wheres(query);

  std::string sql = "update " + table + joins + " set " + columns + " " + wheres;
  rtrim(sql);

  // If the query has an order by clause we will compile it since MySQL supports 
  // order bys on update statements. We'll compile them using the typical way 
  // of compiling order bys. Then they will be appended to the SQL queries.
  if (!query.orders_.empty())
    sql += " " + compile_orders(query, query.orders_);

  // Updates on MySQL also supports "limits", which allow you to easily update a 
  // single record very easily. This is not supported by all database engines
  // so we have customized this update compiler here in order to add it in.
  if (query.limit_ != -1)
    sql += " " + compile_limit(query, query.limit_);

  return rtrim(sql);
}

//Prepare the bindings for an update statement.
MysqlGrammar::variable_array_t MysqlGrammar::prepare_bindings_forupdate(
    db_query_bindings_t &bindings, const variable_array_t &values) {
  auto vals = collect(values).reject([](variable_t &value){
    //Not complete.
    return true;    
  }).all();
  return Grammar::prepare_bindings_forupdate(bindings, vals);
}

//Compile a delete statement into SQL.
std::string MysqlGrammar::compile_delete(Builder &query) {
  auto table = wrap_table(query.from_);
  auto where = !query.wheres_.empty() ? compile_wheres(query) : "";
  if (query.joins_.empty()) {
    return compile_delete_without_joins(query, table, where);
  } else {
    return compile_delete_with_joins(query, table, where);
  }
}

//Compile a single union statement.
std::string MysqlGrammar::compile_union(db_query_array_t &_union) {
  if (is_null(_union.query)) return "";
  std::string conjunction = !empty(_union["all"]) && (_union["all"] == true) ? 
                           " union all " : " union ";
  return conjunction + "(" + _union.query->to_sql() + ")";
}

//Compile the lock into SQL.
std::string MysqlGrammar::compile_lock(
    Builder &query, const variable_t &value) {
  if (kVariableTypeBool == value.type)
    return value == true ? "for update" : "lock in share mode";
  return value.data;
}

//Compile all of the columns for an update statement.
std::string MysqlGrammar::compile_update_columns(variable_set_t &values) {
  variable_array_t r;
  for (auto it = values.begin(); it != values.end(); ++it) {
    if (is_json_selector(it->first)) {
      r.push_back(compile_json_update_column(it->first, it->second.data));
    } else {
      r.push_back(wrap(it->first) + " = " + parameter(it->second));
    }
  }
  return implode(", ", r);
}

//Prepares a JSON column being updated using the JSON_SET function.
std::string MysqlGrammar::compile_json_update_column(
    const std::string &key, const std::string &value) {
  std::vector<std::string> path;
  explode(key.c_str(), path, "->", true, true);
  if (path.empty()) return "";
  auto it = path.begin();
  auto field = *it;
  path.erase(it);
  auto accessor = "\"$." + implode(".", path) + "\"";
  return field + " = json_set(" + field + ", " + accessor + ", " + value;
}

//Compile a delete query that does not use joins.
std::string MysqlGrammar::compile_delete_without_joins(
    Builder &query, const std::string &table, const std::string &where) {
  std::string sql = "delete from " + table + " " + where; 
  trim(sql);

  // When using MySQL, delete statements may contain order by statements and limits 
  // so we will compile both of those here. Once we have finished compiling this
  // we will return the completed SQL statement so it will be executed for us.
  if (!query.orders_.empty())
    sql += " " + compile_orders(query, query.orders_);

  if (query.limit_ != -1)
    sql += " " + compile_limit(query, query.limit_);

  return sql;
}

//Compile a delete query that uses joins.
std::string MysqlGrammar::compile_delete_with_joins(
    Builder &query, const std::string &table, const std::string &where) {
  std::string joins = " " + compile_joins(query, query.joins_);

  std::string alias{table}; 
  if (table.find(" as ") != std::string::npos) {
    std::vector<std::string> array;
    explode(table.c_str(), array, " as ", true, true);
    alias = array[0];
  }
  std::string r = "delete " + alias + " from " + table + joins + " " + where;
  return trim(r);
}
 
//Wrap a single string in keyword identifiers.
std::string MysqlGrammar::wrap_value(const std::string &value) {
  if ("*" == value) return value;

  // If the given value is a JSON selector we will wrap it differently than a 
  // traditional value. We will need to split this path and wrap each part
  // wrapped, etc. Otherwise, we will simply wrap the value as a string.
  if (is_json_selector(value))
    return wrap_json_selector(value);
  
  return "`" + str_replace("`", "``", value) + "`";
}

//Wrap the given JSON selector.
std::string MysqlGrammar::wrap_json_selector(const std::string &value) {
  std::vector<std::string> path;
  explode(value.c_str(), path, "->", true, true);
  if (path.empty()) return "";
  auto field = wrap_table(path[0]);
  path.erase(path.begin());
  auto parts = collect(path).map([] (std::string &part){
    return "\"" + part + "\"";
  }).implode(".");
  return field + "->'$." + parts + "'";
}

//Determine if the given string is a JSON selector.
bool MysqlGrammar::is_json_selector(const std::string &value) const {
  return contains(value, {"->"});
}
