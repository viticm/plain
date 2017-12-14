#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/query/grammars/grammar.h"

using namespace pf_basic::string;
using namespace pf_basic::type;
using namespace pf_support;
using namespace pf_db::query::grammars;

//The construct function.
Grammar::Grammar() {
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
    "unions",
    "lock",
  };
}

//Compile a select query into SQL.
std::string Grammar::compile_select(Builder &query) {
  // If the query does not have any columns set, we'll set the columns to the
  // * character to just get all of the columns from the database. Then we
  // can build the query and concatenate all the pieces together as one.
  auto original = query.columns_;
  
  if (query.columns_.empty()) query.columns_ = {"*"};

  // To compile the query, we'll spin through each component of the query and
  // see if that component exists. If it does we'll just call the compiler
  // function for the component which is responsible for making the SQL.
  std::string sql = concatenate(compile_components(query)); trim(sql);

  query.columns_ = original;

  return sql;
}

//Compile the random statement into SQL.
std::string Grammar::compile_random(const std::string &seed) {

}

//Compile an insert statement into SQL.
std::string Grammar::compile_insert(
    Builder &query, const variable_array_t &values) {

}

//Compile an update statement into SQL.
std::string Grammar::compile_update(
    Builder &query, const variable_array_t &values) {

}

//Prepare the bindings for an update statement.
Grammar::variable_set_t Grammar::prepare_bindings_forupdate(
    const variable_set_t &bindings, const variable_array_t &values) {

}

//Compile a delete statement into SQL.
std::string Grammar::compile_delete(Builder &query) {

}

//Compile an insert and get ID statement into SQL.
std::string Grammar::compile_insert_getid(
    Builder &query, 
    const variable_array_t &values, 
    const std::string &sequence) {

}

//Compile a truncate table statement into SQL.
Grammar::variable_set_t Grammar::compile_truncate(Builder &query) {

}

//Compile a where exists clause.
std::string Grammar::where_exists(
    Builder &query, const variable_set_t &where) {

}

//Determine if the grammar supports savepoints.
bool Grammar::supports_savepoints() const {

}

//Compile the SQL statement to define a savepoint.
std::string Grammar::compile_savepoint(const std::string &name) {

}

//Compile the SQL statement to execute a savepoint rollback.
std::string Grammar::compile_savepoint_rollback(
    const std::string &name) {

}

//Compile the components necessary for a select clause.
Grammar::variable_set_t Grammar::compile_components(Builder &query) {
  variable_set_t sql;
  // To compile the query, we'll spin through each component of the query and 
  // see if that component exists. If it does we'll just call the compiler
  // function for the component which is responsible for making the SQL.
  for (const std::string &component : select_components_)
    sql[component] = call(query, component);
  return sql;
}

//Compile an aggregated select clause.
std::string Grammar::compile_aggregate(
    Builder &query, variable_set_t &aggregate) {
  std::string column = aggregate["columns"].c_str();

  // If the query has a "distinct" constraint and we're not asking for all columns
  // we need to prepend "distinct" onto the column name so that the query takes
  // it into account when it performs the aggregating operations on the data.
  if (query.distinct_ && column != "*") 
    column = "distinct " + column;
  
  return "select " + aggregate["function"].data + "(" + column + ") as aggregate";
}

//Compile the "join" portions of the query.
std::string Grammar::compile_joins(
    Builder &query, const std::vector<JoinClause> &joins) {

  return collect(joins).map([this, &query](JoinClause &join) {
    std::string table = this->wrap_table(join.table_);
    std::string r = join.type_ + " join " + table + this->compile_wheres(join);
    return trim(r);
  }).implode(" ");
}

//Compile the "where" portions of the query.
std::string Grammar::compile_wheres(Builder &query) {
  // Each type of where clauses has its own compiler function which is responsible 
  // for actually creating the where clauses SQL. This helps keep the code nice
  // and maintainable since each clause has a very small method that it uses.
  if (query.wheres_.empty()) return "";

  // If we actually have some where clauses, we will strip off the first boolean
  // operator, which is added by the query builders for convenience so we can 
  // avoid checking for the first clauses in each of the compilers methods.
  auto sql = compile_wheres_toarray(query);

  return sql.size() > 0 ? concatenate_where_clauses(query, sql) : "";
}



//Get an array of all the where clauses for the query.
Grammar::variable_set_t Grammar::compile_wheres_toarray(Builder &query) {

}

//Call the compile method from string.
std::string Grammar::call(Builder &query, const std::string &component) {
  std::string r{""};
  if ("aggregate" == component && !query.aggregate_.empty()) {
    r = compile_aggregate(query, query.aggregate_);
  } else if ("columns" == component && !query.columns_.empty()) {
    r = compile_columns(query, query.columns_);
  } else if ("from" == component && query.from_ != "") {
    r = compile_from(query, query.from_);
  } else if ("joins" == component && !query.joins_.empty()) {
    r = compile_joins(query, query.joins_);
  } else if ("wheres" == component && !query.wheres_.empty()) {
    r = compile_wheres(query);
  } else if ("groups" == component && !query.groups_.empty()) {
    r = compile_groups(query, query.groups_);
  } else if ("havings" == component && !query.havings_.empty()) {
    r = compile_havings(query, query.havings_);
  } else if ("orders" == component && !query.orders_.empty()) {
    r = compile_orders(query, query.orders_);
  } else if ("limit" == component && query.limit_ > 0) {
    r = compile_limit(query, query.limit_);
  } else if ("offset" == component && query.offset_ > 0) {
    r = compile_offset(query, query.offset_);
  } else if ("unions" == component && !query.unions_.empty()) {
    r = compile_unions(query);
  } else if ("lock" == component && query.lock_.type != kVariableTypeInvalid) {
    r = compile_lock(query, query.lock_);
  }
  return r;
}

//Format the where clause statements into one string.
std::string Grammar::concatenate_where_clauses(
    Builder &query, variable_set_t &sql) {

}

//Compile a raw where clause.
std::string Grammar::where_raw(Builder &query, const variable_set_t &where) {
}

//Compile a basic where clause.
std::string Grammar::where_basic(Builder &query, const variable_set_t &where) {

}

//Compile a "where in" clause.
std::string Grammar::where_in(Builder &query, const variable_set_t &where) {

}

//Compile a "where not in" clause.
std::string Grammar::where_notin(Builder &query, const variable_set_t &where) {

}

//Compile a where in sub-select clause.
std::string Grammar::where_insub(Builder &query, const variable_set_t &where) {

}

//Compile a where not in sub-select clause.
std::string Grammar::where_not_insub(
    Builder &query, const variable_set_t &where) {

}

//Compile a "where null" clause.
std::string Grammar::where_null(Builder &query, const variable_set_t &where) {

}

//Compile a "where not null" clause.
std::string Grammar::where_notnull(Builder &query, const variable_set_t &where) {

}

//Compile a "between" where clause.
std::string Grammar::where_between(Builder &query, const variable_set_t &where) {

}

//Compile a "where time" clause.
std::string Grammar::where_time(Builder &query, const variable_set_t &where) {

}

//Compile a where clause comparing two columns.
std::string Grammar::where_column(Builder &query, const variable_set_t &where) {

}

//Compile a nested where clause.
std::string Grammar::where_nested(Builder &query, const variable_set_t &where) {

}

//Compile a where condition with a sub-select.
std::string Grammar::where_sub(Builder &query, const variable_set_t &where) {

}

//Compile a where not exists clause.
std::string Grammar::where_notexists(
    Builder &query, const variable_set_t &where) {

}

//Compile the "group by" portions of the query.
std::string Grammar::compile_groups(
    Builder &query, const std::vector<std::string> &groups) {

}

//Compile the "having" portions of the query.
std::string Grammar::compile_havings(
    Builder &query, const std::vector<std::string> &havings) {

}

//Compile a single having clause.
std::string Grammar::compile_having(const std::vector<std::string> &having) {

}

//Compile a basic having clause.
std::string Grammar::compile_basic_having(const variable_set_t &having) {

}

//Compile the "order by" portions of the query.
std::string Grammar::compile_orders(
    Builder &query, const variable_set_t &orders) {

}

//Compile the query orders to an array.
Grammar::variable_set_t Grammar::compile_orders_toarray(
    Builder &query, const variable_set_t &orders) {

}

//Compile an exists statement into SQL.
std::string Grammar::compile_exists(Builder &query) {

}

//Compile a single union statement.
std::string Grammar::compile_union(const variable_set_t &unions) {

}

//Compile the "union" queries attached to the main query.
std::string Grammar::compile_unions(Builder &query) {

}

//Compile the lock into SQL.
std::string Grammar::compile_lock(Builder &query, const std::string &value) {

}

//Compile a "where day" clause.
std::string Grammar::where_day(Builder &query, const variable_set_t &where) {

}

//Compile a "where month" clause.
std::string Grammar::where_month(Builder &query, const variable_set_t &where) {

}

//Compile a "where year" clause.
std::string Grammar::where_year(Builder &query, const variable_set_t &where) {

}

//Compile a "where date" clause.
std::string Grammar::where_date(Builder &query, const variable_set_t &where) {

}

//Compile a date based where clause.
std::string Grammar::date_based_where(
    const std::string &type, Builder &query, const variable_set_t &where) {

}

//Compile the "select *" portion of the query.
std::string Grammar::compile_columns(
    Builder &query, const std::vector<std::string> &columns) {
  // If the query is actually performing an aggregating select, we will let that
  // compiler handle the building of the select clauses, as it will need some
  // more syntax that is best handled by that function to keep things neat. 
  if (query.aggregate_.empty()) return "";

  std::string select = query.distinct_ ? "select distinct " : "select ";

  return select + columnize(columns);
}

//Compile the "from" portion of the query.
std::string Grammar::compile_from(Builder &query, const std::string &table) {
  return "from " + wrap_table(table);
}

//Compile the "limit" portions of the query.
std::string Grammar::compile_limit(Builder &query, int32_t limit) {

}

//Concatenate an array of segments, removing empties.
std::string Grammar::concatenate(const variable_set_t &segments) {

}

//Remove the leading boolean from a statement.
std::string Grammar::remove_leading_boolean(const std::string &value) {

}
