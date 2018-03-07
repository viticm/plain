#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/db/query/grammars/postgres_grammar.h"

using namespace pf_db::query::grammars;
using namespace pf_basic::string;
using namespace pf_basic::type;
using namespace pf_support;

PostgresGrammar::PostgresGrammar() {
  operators_ = {
    "=", "<", ">", "<=", ">=", "<>", "!=",
    "like", "not like", "between", "ilike",
    "&", "|", "#", "<<", ">>",
    "@>", "<@", "?", "?|", "?&", "||", "-", "-", "#-",
  };
}

PostgresGrammar::~PostgresGrammar() {

}

//Compile an insert statement into SQL.
std::string PostgresGrammar::compile_insert(
    Builder &query, std::vector<variable_set_t> &values) {
  auto table = wrap_table(query.from_);
  return values.empty() ?
         "insert into " + table + " DEFAULT VALUES" :
         Grammar::compile_insert(query, values);
}

//Compile an insert and get ID statement into SQL.
std::string PostgresGrammar::compile_insert_getid(
    Builder &query, 
    std::vector<variable_set_t> &values, 
    const std::string &sequence) {
  return compile_insert(query, values) + " returning " +
         ("" == sequence ? "id" : wrap(sequence));
}

//Compile an update statement into SQL.
std::string PostgresGrammar::compile_update(
    Builder &query, variable_set_t &values) {
  auto table = wrap_table(query.from_);
  // Each one of the columns in the update statements needs to be wrapped in the 
  // keyword identifiers, also a place-holder needs to be created for each of 
  // the values in the list of bindings so we can make the sets statements.
  auto columns = compile_update_columns(values);

  auto from = compile_update_from(query);

  auto where = compile_update_wheres(query);

  std::string sql = "update " + table + " set " + columns + from + " " + where;
  return trim(sql);
}

//Prepare the bindings for an update statement.
variable_array_t PostgresGrammar::prepare_bindings_forupdate(
    db_query_bindings_t &bindings, const variable_array_t &values) {
  // Update statements with "joins" in Postgres use an interesting syntax. We need to 
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
std::string PostgresGrammar::compile_delete(Builder &query) {
  auto table = wrap_table(query.from_);
  return !query.joins_.empty() ?
         compile_delete_with_joins(query, table):
         Grammar::compile_delete(query);
}

//Compile a truncate table statement into SQL.
variable_set_t PostgresGrammar::compile_truncate(Builder &query) {
  std::string key = "truncate " + wrap_table(query.from_) + " restart identity";
  return {{key, ""}};
}

//Compile a "where date" clause.
std::string PostgresGrammar::where_date(
    Builder &query, db_query_array_t &where) {
  auto value = parameter(where["value"]);
  return wrap(where["column"]) + "::date " + where["operator"].data + " " + value;
}

//Compile a date based where clause.
std::string PostgresGrammar::date_based_where(
    const std::string &type, Builder &query, db_query_array_t &where) {
  auto value = parameter(where["value"]);
  return "extract(" + type + " from " + wrap(where["column"]) + 
         ") " + where["operator"].data + " " + value;
}

//Compile the lock into SQL.
std::string PostgresGrammar::compile_lock(
    Builder &query, const variable_t  &value) {
  if (kVariableTypeBool == value.type)
    return value.get<bool>() ? "for update" : "for share";
  return value.data;
}

//Compile the columns for the update statement.
std::string PostgresGrammar::compile_update_columns(
    variable_set_t &values) {
  // When gathering the columns for an update statement, we'll wrap each of the  
  // columns and convert it to a parameter value. Then we will concatenate a
  // list of the columns that can be added into this update query clauses.
  variable_array_t r;
  for (auto it = values.begin(); it != values.end(); ++it)
    r.push_back(wrap(it->first) + " = " + parameter(it->second));

  return implode(", ", r);
}

//Compile the "from" clause for an update with a join.
std::string PostgresGrammar::compile_update_from(Builder &query) {
  if (query.joins_.empty()) return "";
  // When using Postgres, updates with joins list the joined tables in the from 
  // clause, which is different than other systems like MySQL. Here, we will
  // compile out the tables that are joined and add them to a from clause.
  std::vector<std::string> froms;
  for (std::unique_ptr<JoinClause> &join : query.joins_) {
    froms.emplace_back(wrap_table(join->table_));
  }
  return froms.empty() ? "" : " from " + implode(", ", froms);
}

//Compile the additional where clauses for updates with joins.
std::string PostgresGrammar::compile_update_wheres(Builder &query) {
  auto base_wheres = compile_wheres(query);

  if (query.joins_.empty()) return base_wheres;

  // Once we compile the join constraints, we will either use them as the where
  // clause or append them to the existing base where clauses. If we need to
  // strip the leading boolean we will do so when using as the only where.
  auto join_wheres = compile_update_join_wheres(query);

  if ("" == trim(base_wheres))
    return "where " + remove_leading_boolean(join_wheres);

  return base_wheres + " " + join_wheres;
}

//Compile the "join" clause where clauses for an update.
std::string PostgresGrammar::compile_update_join_wheres(Builder &query) {
  std::vector<std::string> join_wheres;

  // Here we will just loop through all of the join constraints and compile them
  // all out then implode them. This should give us "where" like syntax after 
  // everything has been built and then we will join it to the real wheres.
  for (std::unique_ptr<JoinClause> &join : query.joins_) {
    for (db_query_array_t &where : join->wheres_) {
      join_wheres.emplace_back(
          where["boolean"].data + " " + 
          call_where(query, where, where["type"].data));
    }
  }
  return implode(" ", join_wheres);
}

//Compile a delete query that uses joins.
std::string PostgresGrammar::compile_delete_with_joins(
    Builder &query, const std::string &table) {
  std::string _using{" USING "};
  std::vector<std::string> froms;
  for (std::unique_ptr<JoinClause> &join : query.joins_) {
    froms.emplace_back(wrap_table(join->table_));
  }
  _using += implode(", ", froms);
  std::string where = !query.wheres_.empty() ? " " + compile_wheres(query) : "";

  std::string sql = "delete from " + table + _using + where;
  return trim(sql);
}

//Wrap a single string in keyword identifiers.
std::string PostgresGrammar::wrap_value(const variable_t &value) {
  if (value == "*") return value.data;
  // If the given value is a JSON selector we will wrap it differently than a
  // traditional value. We will need to split this path and wrap each part
  // wrapped, etc. Otherwise, we will simply wrap the value as a string.
  if (contains(value.data, {"->"})) return wrap_json_selector(value.data);
  return "\"" + str_replace("\"", "\"", value.data) + "\"";
}

//Wrap the given JSON selector.
std::string PostgresGrammar::wrap_json_selector(const std::string &value) {
  std::vector<std::string> path;
  explode(value.c_str(), path, "->", true, true);
  if (path.empty()) return "";
  auto field = wrap_table(path[0]);
  path.erase(path.begin());
  wrap_json_path_attributes(path);
  auto attribute = path[path.size() - 1];
  path.erase(path.end());
  if (!path.empty())
    return field + "->" + implode("->", path) + "->>" + attribute;
  return field + "->>" + attribute;
}

//Wrap the attributes of the give JSON path.
void PostgresGrammar::wrap_json_path_attributes(std::vector<std::string> &path) {
  for (std::string &item : path)
    item = "'" + item + "'";
}
