/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id grammar.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/04 15:13
 * @uses your description
*/
#ifndef PF_DB_QUERY_GRAMMARS_GRAMMAR_H_
#define PF_DB_QUERY_GRAMMARS_GRAMMAR_H_

#include "pf/db/query/grammars/config.h"
#include "pf/db/grammar.h"
#include "pf/db/query/builder.h"
#include "pf/db/query/join_clause.h"

namespace pf_db {

namespace query {

namespace grammars {

class PF_API Grammar : public pf_db::Grammar {

 public:
   Grammar();
   virtual ~Grammar() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using Builder = pf_db::query::Builder;

 public:
   
   //Compile a select query into SQL.
   virtual std::string compile_select(Builder &query);

   //Compile the random statement into SQL.
   virtual std::string compile_random(const std::string &seed);

   //Compile an insert statement into SQL.
   virtual std::string compile_insert(
       Builder &query, std::vector<variable_set_t> &values);

   //Compile an update statement into SQL.
   virtual std::string compile_update(
       Builder &query, variable_set_t &values);

   //Prepare the bindings for an update statement.
   virtual variable_array_t prepare_bindings_forupdate(
       db_query_bindings_t &bindings, const variable_array_t &values);

   //Compile a delete statement into SQL.
   virtual std::string compile_delete(Builder &query);

   //Compile an insert and get ID statement into SQL.
   virtual std::string compile_insert_getid(
       Builder &query, 
       std::vector<variable_set_t> &values, 
       const std::string &sequence);

   //Compile a truncate table statement into SQL.
   virtual variable_set_t compile_truncate(Builder &query);

   //Compile a where exists clause.
   virtual std::string where_exists(Builder &query, db_query_array_t &where);

   //Determine if the grammar supports savepoints.
   virtual bool supports_savepoints() const;

   //Compile the SQL statement to define a savepoint.
   virtual std::string compile_savepoint(const std::string &name);

   //Compile the SQL statement to execute a savepoint rollback.
   virtual std::string compile_savepoint_rollback(const std::string &name);

   //Call the compile method from string.
   virtual std::string call_compile(Builder &query, const std::string &component);

   //Call the where method from string.
   virtual std::string call_where(
       Builder &query, db_query_array_t &where, const std::string &method);

 public:
 
   //Compile the components necessary for a select clause.
   variable_set_t compile_components(Builder &query);

   //Compile an aggregated select clause.
   std::string compile_aggregate(Builder &query, variable_set_t &aggregate);

    //Compile the "join" portions of the query.
   std::string compile_joins(
       Builder &query, const std::vector<JoinClause> &joins);

   //Compile the "where" portions of the query.
   std::string compile_wheres(Builder &query);

   //Get an array of all the where clauses for the query.
   variable_array_t compile_wheres_toarray(Builder &query);

   //Format the where clause statements into one string.
   std::string concatenate_where_clauses(Builder &query, variable_array_t &sql);

   //Compile a raw where clause.
   std::string where_raw(Builder &query, db_query_array_t &where);

   //Compile a basic where clause.
   std::string where_basic(Builder &query, db_query_array_t &where);

   //Compile a "where in" clause.
   std::string where_in(Builder &query, db_query_array_t &where);

   //Compile a "where not in" clause.
   std::string where_notin(Builder &query, db_query_array_t &where);

   //Compile a where in sub-select clause.
   std::string where_insub(Builder &query, db_query_array_t &where);

   //Compile a where not in sub-select clause.
   std::string where_not_insub(Builder &query, db_query_array_t &where);

   //Compile a "where null" clause.
   std::string where_null(Builder &query, db_query_array_t &where);

   //Compile a "where not null" clause.
   std::string where_notnull(Builder &query, db_query_array_t &where);

   //Compile a "between" where clause.
   std::string where_between(Builder &query, db_query_array_t &where);

   //Compile a "where time" clause.
   std::string where_time(Builder &query, db_query_array_t &where);

    //Compile a where clause comparing two columns.
   std::string where_column(Builder &query, db_query_array_t &where);

   //Compile a nested where clause.
   std::string where_nested(Builder &query, db_query_array_t &where);

   //Compile a where condition with a sub-select.
   std::string where_sub(Builder &query, db_query_array_t &where);

   //Compile a where not exists clause.
   std::string where_notexists(Builder &query, db_query_array_t &where);

   //Compile the "group by" portions of the query.
   std::string compile_groups(
       Builder &query, const std::vector<std::string> &groups);

   //Compile the "having" portions of the query.
   std::string compile_havings(
       Builder &query, const std::vector<variable_set_t> &havings);

   //Compile a single having clause.
   std::string compile_having(variable_set_t &having);

   //Compile a basic having clause.
   std::string compile_basic_having(variable_set_t &having);

   //Compile the "order by" portions of the query.
   std::string compile_orders(
       Builder &query, const std::vector<variable_set_t> &orders);

   //Compile the query orders to an array.
   variable_array_t compile_orders_toarray(
       Builder &query, const std::vector<variable_set_t> &orders);

   //Compile an exists statement into SQL.
   std::string compile_exists(Builder &query);

   //Get the grammar specific operators.
   std::vector<std::string> get_operators() {
     return operators_;
   };

  protected:

   //The grammar specific operators.
   std::vector<std::string> operators_;

   //The components that make up a select clause.
   std::vector<std::string> select_components_;

   //The where hash functions.
   std::map< std::string, 
     std::function<std::string(Builder &, db_query_array_t &)> > where_calls_;

   std::map<std::string, std::string> a_;

 protected:

   //Compile a single union statement.
   virtual std::string compile_union(db_query_array_t &_union);

   //Compile the "union" queries attached to the main query.
   virtual std::string compile_unions(Builder &query);

   //Compile the lock into SQL.
   virtual std::string compile_lock(Builder &query, const variable_t &value);

   //Compile a "where day" clause.
   virtual std::string where_day(Builder &query, db_query_array_t &where);

   //Compile a "where month" clause.
   virtual std::string where_month(Builder &query, db_query_array_t &where);

   //Compile a "where year" clause.
   virtual std::string where_year(Builder &query, db_query_array_t &where);

   //Compile a "where date" clause.
   virtual std::string where_date(Builder &query, db_query_array_t &where);

   //Compile a date based where clause.
   virtual std::string date_based_where(
       const std::string &type, Builder &query, db_query_array_t &where);

   //Compile the "select *" portion of the query.
   virtual std::string compile_columns(
       Builder &query, const std::vector<std::string> &columns);

   //Compile the "from" portion of the query.
   virtual std::string compile_from(Builder &query, const std::string &table);

   //Compile the "limit" portions of the query.
   virtual std::string compile_limit(Builder &query, int32_t limit);

   //Concatenate an array of segments, removing empties.
   virtual std::string concatenate(variable_set_t &segments);

   //Remove the leading boolean from a statement.
   virtual std::string remove_leading_boolean(const std::string &value);

 protected:

   //Compile the "offset" portions of the query.
   std::string compile_offset(Builder &query, int32_t offset);

};

}; //namespace grammars

}; //namespace query

}; //namespace pf_db

#endif //PF_DB_QUERY_GRAMMARS_GRAMMAR_H_
