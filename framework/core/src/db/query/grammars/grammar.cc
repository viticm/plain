#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/concerns/builds_queries.h"
#include "pf/db/query/grammars/grammar.h"

using namespace pf_basic::string;
using namespace pf_basic::type;
using namespace pf_support;
using namespace pf_db::query::grammars;

#define where_call(name) \
  [this](Builder &query, db_query_array_t &where) { \
     return where_##name(query, where); \
  }

#define safe_call_where(name, query, where) \
  (where_calls_.find(name) != where_calls_.end() ? \
   where_calls_[name](query, where) : "")

#define where_query(where) (*(where.query.get()))

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
  where_calls_ = {
    { "raw", where_call(raw) },
    { "basic", where_call(basic) },
    { "in", where_call(in) },
    { "notin", where_call(notin) },
    { "insub", where_call(insub) },
    { "not_insub", where_call(not_insub) },
    { "null", where_call(null) },
    { "notnull", where_call(notnull) },
    { "between", where_call(between) },
    { "time", where_call(time) },
    { "column", where_call(column) },
    { "time", where_call(time) },
    { "nested", where_call(nested) },
    { "sub", where_call(sub) },
    { "exists", where_call(exists) },
    { "notexists", where_call(notexists) },
    { "day", where_call(day) },
    { "month", where_call(raw) },
    { "year", where_call(year) },
    { "date", where_call(date) },
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
  variable_set_t components = compile_components(query);
  std::string sql = concatenate(components); trim(sql);

  query.columns_ = original;

  return sql;
}

//Compile the random statement into SQL.
std::string Grammar::compile_random(const std::string &seed) {
  return "RANDOM()";
}

//Compile an insert statement into SQL.
std::string Grammar::compile_insert(
    Builder &query, std::vector<variable_set_t> &values) {
  if (values.empty()) return "";
  // Essentially we will force every insert to be treated as a batch insert which 
  // simply makes creating the SQL easier for us since we can utilize the same 
  // basic routine regardless of an amount of records given to us to insert.
  auto table = wrap_table(query.from_);
  
  auto columns = columnize(array_keys(values[0]));

  // We need to build a list of parameter place-holders of values that are bound
  // to the query. Each insert should have the exact same amount of parameter
  // bindings so we will loop through the record and parameterize them all.
  auto parameters = collect(values).map([this](variable_set_t &record){
    return "(" + parameterize(record) + ")";
  }).implode(", ");

  return "insert into " + table + "(" + columns + ")" + " values " + parameters;
}

//Compile an update statement into SQL.
std::string Grammar::compile_update(
    Builder &query, variable_set_t &values) {
  if (values.empty()) return "";

  auto table = wrap_table(query.from_);

  // Each one of the columns in the update statements needs to be wrapped in the 
  // keyword identifiers, also a place-holder needs to be created for each of 
  // the values in the list of bindings so we can make the sets statements.
  variable_array_t _columns;
  for (auto it = values.begin(); it != values.end(); ++it) {
    std::string one = wrap(it->first) + " = " + parameter(it->second);
    _columns.push_back(one);
  }

  auto columns = implode(", ", _columns);

  // If the query has any "join" clauses, we will setup the joins on the builder
  // and compile them so we can attach them to this update, as update queries
  // can get join statements to attach to other tables when they're needed.
  std::string joins{""};

  if (!query.joins_.empty()) {
    joins += " " + compile_joins(query, query.joins_);
  }

  // Of course, update queries may also be constrained by where clauses so we'll
  // need to compile the where clauses and attach it to the query so only the
  // intended records are updated by the SQL statements we generate to run.
  auto wheres = compile_wheres(query);

  std::string sql = "update " + table + joins + " set " + columns + " " + wheres;

  return trim(sql);
} 

//Prepare the bindings for an update statement.
Grammar::variable_array_t Grammar::prepare_bindings_forupdate(
    db_query_bindings_t &bindings, const variable_array_t &values) {
  return {};
}

//Compile a delete statement into SQL.
std::string Grammar::compile_delete(Builder &query) {
  std::string wheres = query.wheres_.empty() ? "" : compile_wheres(query);
  std::string r = "delete from " + wrap_table(query.from_) + " " + wheres;
  return trim(r);
}

//Compile an insert and get ID statement into SQL.
std::string Grammar::compile_insert_getid(
    Builder &query, 
    std::vector<variable_set_t> &values, 
    const std::string &sequence) {
  return compile_insert(query, values);
}

//Compile a truncate table statement into SQL.
Grammar::variable_set_t Grammar::compile_truncate(Builder &query) {
  variable_set_t r;
  r["truncate " + wrap_table(query.from_)] = "";
  return r;
}

//Compile a where exists clause.
std::string Grammar::where_exists(
    Builder &query, db_query_array_t &where) {
  if (is_null(where.query)) return "";

  return "exists (" + compile_select(where_query(where)) + ")";
}

//Determine if the grammar supports savepoints.
bool Grammar::supports_savepoints() const {
  return true;
}

//Compile the SQL statement to define a savepoint.
std::string Grammar::compile_savepoint(const std::string &name) {
  return "SAVEPOINT " + name;
}

//Compile the SQL statement to execute a savepoint rollback.
std::string Grammar::compile_savepoint_rollback(
    const std::string &name) {
  return "ROLLBACK TO SAVEPOINT " + name;
}

//Compile the components necessary for a select clause.
Grammar::variable_set_t Grammar::compile_components(Builder &query) {
  variable_set_t sql;
  // To compile the query, we'll spin through each component of the query and 
  // see if that component exists. If it does we'll just call the compiler
  // function for the component which is responsible for making the SQL.
  for (const std::string &component : select_components_)
    sql[component] = call_compile(query, component);
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
    Builder &query, std::vector< std::unique_ptr<JoinClause> > &joins) {

  std::vector<std::string> array;
  for (std::unique_ptr<JoinClause> &join : joins) {
    std::string table = this->wrap_table(join->table_);
    std::string r = join->type_ + " join " + table + 
                    this->compile_wheres(*join.get());
    array.emplace_back(trim(r));
  }
  return implode(" ", array);
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

  return !sql.empty() ? concatenate_where_clauses(query, sql) : "";
}

//Get an array of all the where clauses for the query.
Grammar::variable_array_t Grammar::compile_wheres_toarray(Builder &query) {
  variable_array_t array;
  for (db_query_array_t &where : query.wheres_) {
    std::string r = where["boolean"].data + " " + 
                    safe_call_where(where["type"], query, where);
    array.emplace_back(r);
  }
  return array;
}

//Call the compile method from string.
std::string Grammar::call_compile(Builder &query, const std::string &component) {
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

//Call the where method from string.
std::string Grammar::call_where(
    Builder &query, db_query_array_t &where, const std::string &method) {
  return safe_call_where(method, query, where);
}

//Format the where clause statements into one string.
std::string Grammar::concatenate_where_clauses(
    Builder &query, variable_array_t &sql) {
  std::string conjunction = "JoinClause" == query.class_name_ ? "on" : "where";
  return conjunction + " " + remove_leading_boolean(implode(" ", sql));
}

//Compile a raw where clause.
std::string Grammar::where_raw(Builder &query, db_query_array_t &where) {
  return where["sql"].data;
}

//Compile a basic where clause.
std::string Grammar::where_basic(Builder &query, db_query_array_t &where) {
  auto value = parameter(where["value"]);
  return wrap(where["column"]) + " " + where["operator"].data + " " + value;
}

//Compile a "where in" clause.
std::string Grammar::where_in(Builder &query, db_query_array_t &where) {
  if (!empty(where["values"])) {
    return wrap(where["column"]) + " in (" + parameterize(where.values) + ")";
  }
  return "0 = 1";
}

//Compile a "where not in" clause.
std::string Grammar::where_notin(Builder &query, db_query_array_t &where) {
  if (!empty(where["values"])) {
    return wrap(where["column"]) + " not in (" + parameterize(where.values) + ")";
  }
  return "1 = 1";
}

//Compile a where in sub-select clause.
std::string Grammar::where_insub(Builder &query, db_query_array_t &where) {
  if (is_null(where.query)) return "";
  return wrap(where["column"]) + " in (" + compile_select(where_query(where));
}

//Compile a where not in sub-select clause.
std::string Grammar::where_not_insub(
    Builder &query, db_query_array_t &where) {
  if (is_null(where.query)) return "";
  return wrap(where["column"]) + " not in (" + compile_select(where_query(where));
}

//Compile a "where null" clause.
std::string Grammar::where_null(Builder &query, db_query_array_t &where) {
  return wrap(where["column"]) + " is null";
}

//Compile a "where not null" clause.
std::string Grammar::where_notnull(Builder &query, db_query_array_t &where) {
  return wrap(where["column"]) + " is not null";
}

//Compile a "between" where clause.
std::string Grammar::where_between(Builder &query, db_query_array_t &where) {
  std::string between = empty(where["not"]) ? "not between" : "between";
  return wrap(where["column"]) + " " + between + " ? and ?";
}

//Compile a "where time" clause.
std::string Grammar::where_time(Builder &query, db_query_array_t &where) {
  return date_based_where("time", query, where);
}

//Compile a where clause comparing two columns.
std::string Grammar::where_column(Builder &query, db_query_array_t &where) {
  return wrap(where["first"]) + " " + where["operator"].data + " " + 
         wrap(where["second"]);
}

//Compile a nested where clause.
std::string Grammar::where_nested(Builder &query, db_query_array_t &where) {
  if (is_null(where.query)) return "";
  // Here we will calculate what portion of the string we need to remove. If this
  // is a join clause query, we need to remove the "on" portion of the SQL and
  // if it is a normal query we need to take the leading "where" of queries.
  int32_t offset = "JoinClause" == query.class_name_ ? 3 : 6;

  return "(" + compile_wheres(where_query(where)).substr(offset) + ")";
}

//Compile a where condition with a sub-select.
std::string Grammar::where_sub(Builder &query, db_query_array_t &where) {
  if (is_null(where.query)) return "";
  auto select = compile_select(where_query(where));

  return wrap(where["column"]) + " " + where["operator"].data + "(" + select + 
         ")";
}

//Compile a where not exists clause.
std::string Grammar::where_notexists(
    Builder &query, db_query_array_t &where) {
  if (is_null(where.query)) return "";

  return "not exists (" + compile_select(where_query(where)) + ")";
}

//Compile the "group by" portions of the query.
std::string Grammar::compile_groups(
    Builder &query, const std::vector<std::string> &groups) {
  return "group by " + columnize(groups);
}

//Compile the "having" portions of the query.
std::string Grammar::compile_havings(
    Builder &query, const std::vector<variable_set_t> &havings) {
  auto sql = collect(havings).map([this](variable_set_t &having){
    return compile_having(having);
  }).implode(" ");
  return "having " + remove_leading_boolean(sql);
}

//Compile a single having clause.
std::string Grammar::compile_having(variable_set_t &having) {
  // If the having clause is "raw", we can just return the clause straight away 
  // without doing any more processing on it. Otherwise, we will compile the
  // clause into SQL based on the components that make it up from builder.
  if ("Raw" == having["type"])
    return having["boolean"].data + " " + having["sql"].data;

  return compile_basic_having(having);
}

//Compile a basic having clause.
std::string Grammar::compile_basic_having(variable_set_t &having) {
  auto column = wrap(having["column"]);
  auto param = parameter(having["value"]);
  return having["boolean"].data + " " + column + " " + having["operator"].data + 
         param;
}

//Compile the "order by" portions of the query.
std::string Grammar::compile_orders(
    Builder &query, const std::vector<variable_set_t> &orders) {
  if (orders.empty()) return "";
  return "order by " + implode(", ", compile_orders_toarray(query, orders));
}

//Compile the query orders to an array.
Grammar::variable_array_t Grammar::compile_orders_toarray(
    Builder &query, const std::vector<variable_set_t> &orders) {
  return collect(orders).map([this](variable_set_t &order){
    return !empty(order["sql"]) ? 
           wrap(order["column"]) + " " + order["direction"].data :
           order["sql"].data;
  }).items_;
}

//Compile an exists statement into SQL.
std::string Grammar::compile_exists(Builder &query) {
  auto select = compile_select(query);
  return "select exists(" + select + ") as " + wrap("exists");
}

//Compile a single union statement.
std::string Grammar::compile_union(db_query_array_t &_union) {
  if (is_null(_union.query)) return "";
  std::string conjunction = !empty(_union["all"]) && (_union["all"] == true) ? 
                           " union all " : " union ";
  return conjunction + _union.query->to_sql();
}

//Compile the "union" queries attached to the main query.
std::string Grammar::compile_unions(Builder &query) {
  std::string sql{""};
  for (db_query_array_t &_union : query.unions_) {
    sql += compile_union(_union);
  }
  if (!query.union_orders_.empty())
    sql += compile_orders(query, query.orders_);
  if (query.union_limit_ != -1)
    sql += compile_limit(query, query.union_limit_);
  if (query.union_offset_ != -1)
    sql += compile_offset(query, query.union_offset_);
  return ltrim(sql);
}

//Compile the lock into SQL.
std::string Grammar::compile_lock(Builder &query, const variable_t &value) {
  return value.data;
}

//Compile a "where day" clause.
std::string Grammar::where_day(Builder &query, db_query_array_t &where) {
  return date_based_where("day", query, where);
}

//Compile a "where month" clause.
std::string Grammar::where_month(Builder &query, db_query_array_t &where) {
  return date_based_where("month", query, where);
}

//Compile a "where year" clause.
std::string Grammar::where_year(Builder &query, db_query_array_t &where) {
  return date_based_where("year", query, where);
}

//Compile a "where date" clause.
std::string Grammar::where_date(Builder &query, db_query_array_t &where) {
  return date_based_where("date", query, where);
}

//Compile a date based where clause.
std::string Grammar::date_based_where(
    const std::string &type, Builder &query, db_query_array_t &where) {
  std::string value = parameter(where["value"]);
  return type + "(" + wrap(where["column"]) + ")" + 
         where["operator"].data + " " + value; 
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
  return "limit " + std::to_string(limit);
}

//Concatenate an array of segments, removing empties.
std::string Grammar::concatenate(variable_set_t &segments) {
  if (segments.empty()) return "";
  variable_array_t r;
  for (auto it = segments.begin(); it != segments.end(); ++it)
    if (it->second != "") r.push_back(it->second);
  return implode(" ", r);
}

//Remove the leading boolean from a statement.
std::string Grammar::remove_leading_boolean(const std::string &value) {
  const std::string _and{"and "};
  const std::string _or{"or "};
  std::string r{value};
  auto find_and = r.find(_and);
  if (find_and != std::string::npos) r.replace(find_and, _and.length(), "");
  auto find_or = r.find(_or);
  if (find_or != std::string::npos) r.replace(find_or, _or.length(), "");
  return r;
}

//Compile the "offset" portions of the query.
std::string Grammar::compile_offset(Builder &query, int32_t offset) {
  return "offset " + std::to_string(offset);
}
