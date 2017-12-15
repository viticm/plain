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

namespace pf_db {

namespace query {

class PF_API Builder {

 public:
   Builder(ConnectionInterface *connection, grammars::Grammar *grammar);
   virtual ~Builder();

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using closure_t = pf_basic::type::closure_t;

 public:

   //The class name.
   std::string class_name_;

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
   std::vector<JoinClause> joins_;

   //The where constraints for the query.
   std::vector<db_query_where_t> wheres_;

   //The groupings for the query.
   std::vector<std::string> groups_;

   //The having constraints for the query.
   std::vector<std::string> havings_;

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
   variable_t lock_;

   //All of the available clause operators. 
   std::vector<std::string> operators_;

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

   //Add a "where time" statement to the query.
   Builder &where_time(const std::string &column, 
                       const std::string &oper, 
                       int32_t value, 
                       const std::string &boolean = "and");

   //Add an "or where time" statement to the query.
   Builder &or_where_time(const std::string &column, 
                          const std::string &oper, 
                          int32_t value);

   //Add a "where day" statement to the query.
   Builder &where_time(const std::string &column, 
                       const std::string &oper, 
                       const variable_t &value = "", 
                       const std::string &boolean = "and");

   //Add a "where month" statement to the query.
   Builder &where_month(const std::string &column, 
                        const std::string &oper, 
                        const variable_t &value = "", 
                        const std::string &boolean = "and");

   //Add a "where year" statement to the query.
   Builder &where_year(const std::string &column, 
                       const std::string &oper, 
                       const variable_t &value = "", 
                       const std::string &boolean = "and");

   //Add a nested where statement to the query.
   Builder &where_nested(closure_t callback, const std::string &boolean = "and");

   //Create a new query instance for nested where condition.
   Builder &for_nested_where();

   //Add another query builder as a nested where to the query builder.
   Builder &add_nested_where_query(Builder &query,
                                   const std::string &boolean = "and");

   //Add an exists clause to the query.
   Builder &where_exists(closure_t callback, 
                         const std::string &boolean = "and", 
                         bool isnot = false);

   //Add an or exists clause to the query.
   Builder &or_where_exists(closure_t callback, bool isnot = false);

   //Add a where not exists clause to the query.
   Builder &where_not_exists(closure_t callback, bool isnot = false);

   //Add a or where not exists clause to the query.
   Builder &or_where_not_exists(closure_t callback, bool isnot = false);

   //Add an exists clause to the query.
   Builder &add_where_exists_query(Builder &query, 
                                   const std::string &boolean = "and",
                                   bool isnot = false);

   //Handles dynamic "where" clauses to the query.
   Builder &dynamic_where(const std::string &method,
                          const std::string &parameters);

   //Add a "group by" clause to the query.
   Builder &group_by(std::vector<std::string> groups);

   //Add a "group by" clause to the query.
   template <typename... TS>
   Builder &group_by(const std::string &param, TS... args);

   //Add a "having" clause to the query.
   Builder &having(const std::string &column, 
                   const std::string &oper = "", 
                   const std::string &value = "", 
                   const std::string &boolean = "and");

   //Add a "or having" clause to the query.
   Builder &or_having(const std::string &column, 
                      const std::string &oper = "", 
                      const std::string &value = "");

   //Add a raw having clause to the query.
   Builder &having_raw(const std::string &sql,
                       const variable_set_t &bindings = {},
                       const std::string &boolean = "and");

   //Add a raw or having clause to the query.
   Builder &or_having_raw(const std::string &sql,
                          const variable_set_t &bindings = {});

   //Add an "order by" clause to the query.
   Builder &order_by(const std::string &column, 
                     const std::string &direction = "asc");

   //Add a descending "order by" clause to the query.
   Builder &order_bydesc(const std::string &column);
  
   //Add an "order by" clause for a timestamp to the query.
   Builder &latest(const std::string &column = "created_at");

   //Add an "order by" clause for a timestamp to the query.
   Builder &oldest(const std::string &column = "created_at");

   //Put the query's results in random order.
   Builder &in_random_order(const std::string &seed = "");

   //Add a raw "order by" clause to the query.
   Builder &order_byraw(const std::string &sql,
                        const variable_set_t &bindings = {});

   //Alias to set the "offset" value of the query.
   Builder &skip(int32_t value);

   //Set the "offset" value of the query.
   Builder &offset(int32_t value);

   //Alias to set the "limit" value of the query.
   Builder &take(int32_t value);

   //Set the "limit" value of the query.
   Builder &limit(int32_t value);

   //Set the limit and offset for a given page.
   Builder &for_page(int32_t page, int32_t perpage);

   //Constrain the query to the next "page" of results after a given ID.
   Builder &for_page_afterid(int32_t perpage, 
                             int32_t lastid, 
                             const std::string &column = "id");

   //Add a union statement to the query.
   Builder &unions(Builder &query, bool all = false);

   //Add a union all statement to the query.
   Builder &union_all(Builder &query);

   //Lock the selected rows in the table.
   Builder &lock(const variable_t &value = true);

   //Lock the selected rows in the table for updating.
   Builder &lock_forupdate();

   //Share lock the selected rows in the table.
   Builder &shared_lock();

   //Get the SQL representation of the query.
   const std::string tosql() const;

   //Execute a query for a single record by ID.
   void find(int32_t id, 
             db_fetch_array_t &result, 
             const std::vector<std::string> columns = {"*"});

   //Get a single column's value from the first result of a query.
   const variable_t value(const std::string &column) const;

   //Execute the query as a "select" statement.
   void get(db_fetch_array_t &result,
            const std::vector<std::string> &columns = {"*"});

   //Run the query as a "select" statement against the connection.
   void run_select(db_fetch_array_t &result);

   //* Get a generator for the given query.
   void cursor();

   //Chunk the results of a query by comparing numeric IDs.
   bool chunk_byid(int32_t count, 
                   closure_t callback, 
                   const std::string &column = "id", 
                   const std::string &alias = "");

   //Throw an exception if the query doesn't have an orderBy clause.
   void enforce_order_by();

   //Get an array with the values of a given column.
   void pluck(const std::string &column, 
              db_fetch_array_t &result, 
              const std::string &key = "");

   //Concatenate values of a given column as a string.
   const std::string implode(
       const std::string &column, const std::string &glue) const;

   //Determine if any rows exist for the current query.
   bool exists();

   //Retrieve the "count" result of the query.
   int32_t count(const std::string &columns = "*");

   //Retrieve the minimum value of a given column.
   const variable_t _min(const std::string &column) const;

   //Retrieve the sum of the values of a given column.
   const variable_t _max(const std::string &column) const;

   //Retrieve the sum of the values of a given column.
   const variable_t sum(const std::string &column) const;

   //Retrieve the average of the values of a given column.
   const variable_t avg(const std::string &column) const;

   //Alias for the "avg" method.
   const variable_t average(const std::string &column) const;

   //Execute an aggregate function on the database.
   void aggregate(const std::string &function, 
                  variable_set_t &result, 
                  const std::vector<std::string> &columns = {"*"});

   //Execute a numeric aggregate function on the database.
   const variable_t numeric_aggregate(
       const std::string &function, 
       const std::vector<std::string> &columns = {"*"});

   //Insert a new record into the database.
   bool insert(const variable_array_t &values);

   //Insert a new record and get the value of the primary key.
   int32_t insert_getid(const variable_array_t &values, 
                        const std::string &sequence = "");

   //Update a record in the database.
   int32_t update(const variable_array_t &values);

   //Insert or update a record matching the attributes, and fill it with values.
   bool update_or_insert(const std::vector<std::string> &attributes,
                         const variable_array_t &values);

   //Increment a column's value by a given amount.
   int32_t increment(const std::string &column, 
                     int32_t amount = 1, 
                     const variable_array_t &extra = {});

   //Decrement a column's value by a given amount.
   int32_t decrement(const std::string &column, 
                     int32_t amount = 1, 
                     const variable_array_t &extra = {});

   //Delete a record from the database.
   int32_t deleted(const variable_t &id = "");

   //Run a truncate statement on the table.
   void truncate();

   //Get a new instance of the query builder.
   Builder new_query();

   //* Create a raw database expression.
   void raw();

   //Get the current query value bindings in a flattened array.
   variable_set_t *get_bindings();

   //Get the raw array of bindings.
   variable_set_t *get_raw_bindings();

   //Set the bindings on the query builder.
   Builder &set_bindings(const variable_set_t &bindings, 
                         const std::string &type = "where");

   //Add a binding to the query.
   Builder &add_binding(const variable_t &value, 
                        const std::string &type = "where");

   //Merge an array of bindings into our bindings.
   Builder &merge_bindings(Builder &query);

   //Remove all of the expressions from a list of bindings.
   void clean_bindings(variable_set_t &bindings);

   //Get the database connection instance.
   ConnectionInterface *get_connection();

   //Get the query grammar instance.
   grammars::Grammar *get_grammar();

   //Clone the query without the given properties.
   Builder clone_without();

   //Clone the query without the given bindings.
   Builder clone_without_bindings(const std::vector<std::string> &except);

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
   
   //Add a date based (year, month, day, time) statement to the query.
   Builder &add_date_based_where(const std::string &type,
                                 const std::string &column,
                                 const std::string &oper,
                                 int32_t value,
                                 const std::string &boolean = "and");

   //Add a full sub-select to the query.
   Builder &where_sub(const std::string &column, 
                      const std::string &oper,
                      closure_t callback,
                      const std::string &boolean);

   //Add a single dynamic where clause statement to the query.
   void add_dynamic(const std::string &segment,
                    const std::string &connector,
                    const variable_array_t &parameters,
                    int32_t index);

   //Get an array orders with all orders for an given column removed.
   void remove_existing_orders_for(const std::string &column, 
                                   std::vector<std::string> &result);

   //Remove the column aliases since they will break count queries.
   void without_select_aliases(std::vector<std::string> &columns);

   //Strip off the table name or alias from a column identifier.
   const std::string strip_table_for_pluck(const std::string &column);

   //Set the aggregate property without running the query.
   Builder &set_aggregate(const std::string &function, 
                          const std::vector<std::string> &columns);
};

}; //namespace query

}; //namespace pf_db

#endif //PF_DB_QUERY_BUILDER_H_
