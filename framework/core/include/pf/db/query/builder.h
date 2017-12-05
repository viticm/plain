/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id builder.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/11/30 15:06
 * @uses The query builder class.
*/
#ifndef PF_DB_QUERY_BUILDER_H_
#define PF_DB_QUERY_BUILDER_H_

#include "pf/db/query/config.h"
#include "pf/db/connection_interface.h"
#include "pf/db/query/grammars/grammar.h"

namespace pf_db {

namespace query {

class Builder {

 public:
   Builder(ConnectionInterface *connection, grammars::Grammar *grammar);
   virtual ~Builder();

 public:

   //Set the columns to be selected.
   Builder &select(const std::vector<std::string> &columns = {"*"});

   //Set the columns to be selected.
   template <typename... TS>
   Builder &select(const std::string &param, TS... args);

   //Add a new "raw" select expression to the query.
   Builder &select_raw(
       const std::string &expression, const variable_set_t &bindings);

   //Add a subselect expression to the query.
   Builder &select_sub(Builder &query, const std::string &as);

   //Add a subselect expression to the query.
   Builder &select_sub(const std::string &query, const std::string &as);

   //Parse the sub-select query into SQL and bindings.
   variable_set_t parse_subselect(Builder &query);

   //Parse the sub-select query into SQL and bindings.
   variable_set_t parse_subselect(const std::string &query);

   //Add a new select column to the query.
   Builder &add_select(const std::vector<std::string> &column);

   //Add a new select column to the query.
   template <typename... TS>
   Builder &add_select(const std::string &param, TS... args);

   //Force the query to only return distinct results.
   Builder &distinct();

   //Set the table which the query is targeting.
   Builder &from(const std::string &table);

   //Add a join clause to the query.
   Builder &join(const std::string &table, 
                 const std::string &first, 
                 const std::string &oper = "", 
                 const std::string &second = "", 
                 const std::string &type = "inner", 
                 bool where = false);

   //Add a "join where" clause to the query.
   Builder &join_where(const std::string &table, 
                       const std::string &first, 
                       const std::string &oper = "", 
                       const std::string &second = "", 
                       const std::string &type = "inner");

   //Add a left join to the query.
   Builder &left_join(const std::string &table,
                      const std::string &first,
                      const std::string &oper = "",
                      const std::string &second = "");

   //Add a "join where" clause to the query.
   Builder &left_join_where(const std::string &table,
                            const std::string &first,
                            const std::string &oper,
                            const std::string &second);

   //Add a right join to the query.
   Builder &right_join(const std::string &table,
                       const std::string &first,
                       const std::string &oper = "",
                       const std::string &second = "");
  
   //Add a "right join where" clause to the query.
   Builder &right_join_where(const std::string &table,
                             const std::string &first,
                             const std::string &oper,
                             const std::string &second);

   //Add a "cross join" clause to the query.
   Builder &cross_join(const std::string &table, 
                       const std::string &first = "", 
                       const std::string &oper = "", 
                       const std::string &second = "");

   //Pass the query to a given callback.
   Builder &tap(closure_t callback);

   //Merge an array of where clauses and bindings.
   void merge_wheres(const variable_set_t &wheres, 
                     const variable_set_t &bindings);

   //Add a basic where clause to the query.
   Builder &where(const std::string &column, 
                  const std::string &oper = "", 
                  const variable_t &value = "", 
                  const std::string &boolean = "and");

   //Add a basic where clause to the query.
   Builder &where(const std::vector<std::string> &column, 
                  const std::string &oper = "", 
                  const variable_t &value = "", 
                  const std::string &boolean = "and");
 
   //Add an array of where clauses to the query.
   Builder &add_array_of_wheres(const variable_set_t &column,
                                const std::string &boolean,
                                const std::string &method = "where");

   //Add an "or where" clause to the query.
   Builder &or_where(const std::string &column, 
                     const std::string &oper, 
                     variable_t value);

   //Add a "where" clause comparing two columns to the query.
   Builder &where_column(const std::string &first, 
                         const std::string &oper = "", 
                         const std::string &second = "", 
                         const std::string &boolean = "and");

   //Add a "where" clause comparing two columns to the query.
   Builder &where_column(const std::vector<std::string> &first, 
                         const std::string &oper = "", 
                         const std::string &second = "", 
                         const std::string &boolean = "and");


   //Add an "or where" clause comparing two columns to the query.
   Builder &or_where_column(const std::string &first,
                            const std::string &oper = "",
                            const std::string &second = "");

   //Add an "or where" clause comparing two columns to the query.
   Builder &or_where_column(const std::vector<std::string> &first,
                            const std::string &oper = "",
                            const std::string &second = "");

   //Add a raw where clause to the query.
   Builder &where_raw(const std::string &sql, 
                      const variable_set_t &bindings = {}, 
                      const std::string &boolean = "and");

   //Add a raw or where clause to the query.
   Builder &or_where_raw(const std::string &sql, 
                         const variable_set_t &bindings = {});

   //Add a "where in" clause to the query.
   Builder &where_in(const std::string &column, 
                     const std::vector<variable_t> &values, 
                     const std::string &boolean = "and", 
                     bool isnot = false);

   //Add an "or where in" clause to the query.
   Builder &or_where_in(const std::string &column, 
                        const std::vector<variable_t> &values);

   //Add a "where not in" clause to the query.
   Builder &where_notin(const std::string &column,
                        const std::vector<variable_t> &values,
                        const std::string &boolean = "and");

   //Add an "or where not in" clause to the query.
   Builder &or_where_notin(const std::string &column,
                           const std::vector<variable_t> &values);

   //Add a "where null" clause to the query.
   Builder &where_null(const std::string &column, 
                       const std::string &boolean = "and", 
                       bool isnot = false);

   //Add an "or where null" clause to the query.
   Builder &or_where_null(const std::string &column);

   //Add a "where not null" clause to the query.
   Builder &where_notnull(const std::string &column, 
                          const std::string &boolean = "and");

   //Add a where between statement to the query.
   Builder &where_between(const std::string &column,
                          const std::vector<variable_t> &values,
                          const std::string &boolean = "and",
                          bool isnot = false);

   //Add an or where between statement to the query.
   Builder &or_where_between(const std::string &column,
                             const std::vector<variable_t> &values);

   //Add a where not between statement to the query.
   Builder &where_notbetween(const std::string &column,
                             const std::vector<variable_t> &values,
                             const std::string &boolean = "and");

   //Add an or where not between statement to the query.
   Builder &or_where_notbetween(const std::string &column,
                                const std::vector<variable_t> &values);

   //Add an "or where not null" clause to the query.
   Builder &or_where_notnull(const std::string &column);

   //Add a "where date" statement to the query.
   Builder &where_date(const std::string &column,
                       const std::string &oper,
                       const variable_t &value = "",
                       const std::string &boolean = "and");

   //Add an "or where date" statement to the query.
   Builder &or_where_date(const std::string &column,
                          const std::string &oper,
                          const std::string &value);

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using closure_t = pf_basic::type::closure_t;
   using Builder = pf_db::query::Builder;

 protected:

   //The database connection instance.
   std::unique_ptr<pf_db::ConnectionInterface> connection_;

   //The database query grammar instance.
   std::unique_ptr<pf_db::query::grammars::Grammar> grammar_;

   //The current query value bindings.
   variable_set_t bindings_;

   //An aggregate function and column to be run.
   variable_set_t aggregate_;

   //The columns that should be returned.
   std::vector<std::string> columns_;

   //Indicates if the query returns distinct results.
   bool distinct_;

   //The table which the query is targeting.
   std::string from_;

   //The table joins for the query.
   std::string<std::string> joins_;

   //The where constraints for the query.
   variable_set_t wheres_;

   //The groupings for the query.
   variable_set_t groups_;

   //The having constraints for the query.
   variable_set_t havings_;

   //The orderings for the query.
   variable_set_t orders_;

   //The maximum number of records to return.
   int32_t limit_;

   //The number of records to skip.
   int32_t offset_;

   //The query union statements.
   variable_set_t unions_;

   //The maximum number of union records to return.
   int32_t union_limit_;

   //The number of union records to skip.
   int32_t union_offset_;

   //The orderings for the union query.
   variable_set_t union_orders_;

   //Indicates whether row locking is being used.
   bool lock_;

   //All of the available clause operators. 
   std::vector<std::string> operators_;

 protected:

   //Prepare the value and operator for a where clause.
   variable_array_t prepare_value_and_operator(
       const std::string &value, 
       const std::string &oper, 
       bool use_default = false);

   //Determine if the given operator and value combination is legal.
   // - Prevents using Null values with invalid operators.
   bool invalid_operator_and_value(const std::string &oper, variable_t value);

   //Determine if the given operator is supported.
   bool invalid_operator(const std::string &oper);

   //Add a where in with a sub-select to the query.
   Builder &where_insub(const std::string &column, 
                        closure_t callback, 
                        const std::string &boolean, 
                        bool isnot);

   //Add an external sub-select to the query.
   Builder &where_in_existing_query(const std::string &column,
                                    Builder &query,
                                    const std::string &boolean,
                                    bool isnot);



};

}; //namespace query

}; //namespace pf_db

#endif //PF_DB_QUERY_BUILDER_H_
