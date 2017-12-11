#include "pf/db/query/grammars/grammar.h"

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

}

//Compile an aggregated select clause.
std::string Grammar::compile_aggregate(
    Builder &query, const variable_set_t &aggregate) {

}

//Compile the "join" portions of the query.
std::string Grammar::compile_joins(
    Builder &query, const std::vector<std::string> &joins) {

}

//Compile the "where" portions of the query.
std::string Grammar::compile_wheres(Builder &query) {

}

//Get an array of all the where clauses for the query.
Grammar::variable_set_t Grammar::compile_wheres_toarray(Builder &query) {

}

//Format the where clause statements into one string.
std::string Grammar::concatenate_where_clauses(Builder &query) {

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
    Builder &query, const std::vector<std::string> &orders) {

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

}

//Compile the "from" portion of the query.
std::string Grammar::compile_from(Builder &query, const std::string &table) {

}

//Compile the "limit" portions of the query.
std::string Grammar::compile_limit(Builder &query, int32_t limit) {

}

//Concatenate an array of segments, removing empties.
std::string Grammar::concatenate(const std::vector<std::string> &segments) {

}

//Remove the leading boolean from a statement.
std::string Grammar::remove_leading_boolean(const std::string &value) {

}
