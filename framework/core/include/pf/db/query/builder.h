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
#include "pf/db/concerns/builds_queries.h"
#include "pf/support/helpers.h"

namespace pf_db {

namespace query {

class PF_API Builder : public concerns::BuildsQueries {

 public:
   Builder(ConnectionInterface *connection, 
           grammars::Grammar *grammar = nullptr);
   virtual ~Builder();

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using closure_t = std::function<void (Builder *)>;

 public:

   //The class name.
   std::string class_name_;

   //The database connection instance.
   pf_db::ConnectionInterface *connection_;

   //The database query grammar instance.
   pf_db::query::grammars::Grammar *grammar_;

   //The current query val bindings.
   db_query_bindings_t bindings_;

   //An aggregate function and column to be run.
   variable_set_t aggregate_;

   //The columns that should be returned.
   std::vector<std::string> columns_;

   //Indicates if the query returns distinct results.
   bool distinct_;

   //The table which the query is targeting.
   std::string from_;

   //The table joins for the query.
   std::vector< std::unique_ptr<JoinClause> > joins_;

   //The where constraints for the query.
   std::vector<db_query_array_t> wheres_;

   //The groupings for the query.
   std::vector<std::string> groups_;

   //The having constraints for the query.
   std::vector<variable_set_t> havings_;

   //The orderings for the query.
   std::vector<variable_set_t> orders_;

   //The maximum number of records to return.
   int32_t limit_;

   //The number of records to skip.
   int32_t offset_;

   //The query union statements.
   std::vector<db_query_array_t> unions_;

   //The maximum number of union records to return.
   int32_t union_limit_;

   //The number of union records to skip.
   int32_t union_offset_;

   //The orderings for the union query.
   std::vector<variable_set_t> union_orders_;

   //Indicates whether row locking is being used.
   variable_t lock_;

   //All of the available clause operators. 
   std::vector<std::string> operators_;

 public:

   //Clear the all members.
   Builder &clear();

   //Set the columns to be selected.
   Builder &select(const std::vector<std::string> &columns = {"*"});

   //Set the columns to be selected.
   /**
   template <typename... TS>
   Builder &select(const std::string &param, TS... args) {
     std::vector<std::string> columns;
     columns.push_back(param);
     pf_support::collectargs(columns, args...);
     return select(columns);
   }
   **/

   //Add a new "raw" select expression to the query.
   Builder &select_raw(
       const std::string &expression, const variable_array_t &bindings = {});

   //Add a subselect expression to the query.
   Builder &select_sub(Builder &query, const std::string &as);

   //Add a subselect expression to the query.
   Builder &select_sub(
       std::function<void(Builder *)> callback, const std::string &as);

   //Parse the sub-select query into SQL and bindings.
   void parse_subselect(
       Builder &query, std::string &sql, variable_array_t &bindings);

   //Parse the sub-select query into SQL and bindings.
   void parse_subselect(
       const std::string &query, std::string &sql, variable_array_t &bindings);

   //Add a new select column to the query.
   Builder &add_select(const std::vector<std::string> &column);

   //Add a new select column to the query.
   /**
   template <typename... TS>
   Builder &add_select(const std::string &param, TS... args) {
     std::vector<std::string> columns;
     columns.push_back(param);
     pf_support::collectargs(columns, args...);
     return add_select(columns);
   }
   **/

   //Force the query to only return distinct results.
   Builder &distinct(){
     distinct_ = true;
     return *this;
   };

   //Set the table which the query is targeting.
   Builder &from(const std::string &table){
     from_ = table;
     return *this;
   };

   //Add a join clause to the query.
   Builder &join(const std::string &table, 
                 const std::string &first, 
                 const std::string &oper = "", 
                 const std::string &second = "", 
                 const std::string &type = "inner", 
                 bool where = false);

   //Add a join clause to the query.
   Builder &join(const std::string &table, 
                 std::function<void(Builder *)> callback,
                 const std::string &oper = "", 
                 const std::string &second = "", 
                 const std::string &type = "inner", 
                 bool where = false);

   //Add a "join where" clause to the query.
   Builder &join_where(const std::string &table, 
                       const std::string &_first, 
                       const std::string &oper = "", 
                       const std::string &second = "", 
                       const std::string &type = "inner") {
     return join(table, _first, oper, second, type, true);
   };

   //Add a left join to the query.
   Builder &left_join(const std::string &table,
                      const std::string &_first,
                      const std::string &oper = "",
                      const std::string &second = "") {
     return join(table, _first, oper, second, "left");
   };

   //Add a "join where" clause to the query.
   Builder &left_join_where(const std::string &table,
                            const std::string &_first,
                            const std::string &oper,
                            const std::string &second) {
     return join_where(table, _first, oper, second, "left");
   };

   //Add a right join to the query.
   Builder &right_join(const std::string &table,
                       const std::string &_first,
                       const std::string &oper = "",
                       const std::string &second = "") {
     return join(table, _first, oper, second, "right");
   };
  
   //Add a "right join where" clause to the query.
   Builder &right_join_where(const std::string &table,
                             const std::string &_first,
                             const std::string &oper,
                             const std::string &second) {
     return join_where(table, _first, oper, second, "right");
   };

   //Add a "cross join" clause to the query.
   Builder &cross_join(const std::string &table, 
                       const std::string &_first = "", 
                       const std::string &oper = "", 
                       const std::string &second = "");

   //Pass the query to a given callback.
   Builder &tap(closure_t callback) {
     callback(this);
     return *this;
   };

   //Merge an array of where clauses and bindings.
   void merge_wheres(std::vector<db_query_array_t> &wheres, 
                     variable_array_t &bindings);

   //Add a basic where clause to the query.
   Builder &where(const std::string &column, 
                  const variable_t &oper = "", 
                  const variable_t &val = "", 
                  const std::string &boolean = "and");

   //Add a basic where clause to the query.
   Builder &where(const std::vector<variable_array_t> &columns, 
                  const std::string &boolean = "and") {
     return add_array_of_wheres(columns, boolean);
   };

   //Add a basic where clause to the query.
   Builder &where(variable_set_t &columns, 
                  const std::string &boolean = "and") {
     return add_array_of_wheres(columns, boolean);
   };

   //Add a basic where clause to the query.
   Builder &where(closure_t column, 
                  const variable_t &oper = "", 
                  const variable_t &val = "", 
                  const std::string &boolean = "and") {
     UNUSED(oper);
     UNUSED(val);
     // If the columns is actually a Closure instance, we will assume the developer
     // wants to begin a nested where statement which is wrapped in parenthesis.
     // We'll add that Closure to the query then return back out immediately.
     return where_nested(column, boolean);
   };

   //Add a basic where clause to the query.
   Builder &where(const std::string &column, 
                  const variable_t &oper, 
                  closure_t val,
                  const std::string &boolean = "and");
 
   //Add an array of where clauses to the query.
   Builder &add_array_of_wheres(const std::vector<variable_array_t> &columns,
                                const std::string &boolean,
                                const std::string &method = "where");

   //Add an array of where clauses to the query.
   Builder &add_array_of_wheres(variable_set_t &columns,
                                const std::string &boolean,
                                const std::string &method = "where");

   //Add an "or where" clause to the query.
   Builder &or_where(const std::string &column, 
                     const std::string &oper, 
                     const variable_t &val) {
     return where(column, oper, val, "or");
   };

   //Add a "where" clause comparing two columns to the query.
   Builder &where_column(const std::string &first, 
                         const std::string &oper = "", 
                         const std::string &second = "", 
                         const std::string &boolean = "and");

   //Add a "where" clause comparing two columns to the query.
   Builder &where_column(const std::vector<variable_array_t> &_first, 
                         const std::string &oper = "", 
                         const std::string &second = "", 
                         const std::string &boolean = "and") {
     UNUSED(oper);
     UNUSED(second);
     // If the column is an array, we will assume it is an array of key-val pairs 
     // and can add them each as a where clause. We will maintain the boolean we
     // received when the method was called and pass it into the nested where.
     return add_array_of_wheres(_first, boolean, "where_column");
   };

   //Add a "where" clause comparing two columns to the query.
   Builder &where_column(variable_set_t &_first, 
                         const std::string &oper = "", 
                         const std::string &second = "", 
                         const std::string &boolean = "and") {
     UNUSED(oper);
     UNUSED(second);
     // If the column is an array, we will assume it is an array of key-val pairs 
     // and can add them each as a where clause. We will maintain the boolean we
     // received when the method was called and pass it into the nested where.
     return add_array_of_wheres(_first, boolean, "where_column");
   };

   //Add an "or where" clause comparing two columns to the query.
   Builder &or_where_column(const std::string &_first,
                            const std::string &oper = "",
                            const std::string &second = "") {
     return where_column(_first, oper, second, "or");
   };

   //Add a "or where" clause comparing two columns to the query.
   Builder &or_where_column(const std::vector<variable_array_t> &_first, 
                            const std::string &oper = "", 
                            const std::string &second = "") {
     return where_column(_first, oper, second, "or");
   };

   //Add a "or where" clause comparing two columns to the query.
   Builder &where_column(variable_set_t &_first, 
                         const std::string &oper = "", 
                         const std::string &second = "") {
     return where_column(_first, oper, second, "or");
   };

   //Add a raw where clause to the query.
   Builder &where_raw(const std::string &sql, 
                      const variable_array_t &bindings, 
                      const std::string &boolean = "and");

   //Add a raw or where clause to the query.
   Builder &or_where_raw(const std::string &sql, 
                         const variable_array_t &bindings) {
     return where_raw(sql, bindings, "or");
   };

   //Add a "where in" clause to the query.
   Builder &where_in(const std::string &column, 
                     const variable_array_t &vals, 
                     const std::string &boolean = "and", 
                     bool isnot = false);

   //Add a "where in" clause to the query.
   Builder &where_in(const std::string &column, 
                     Builder &query, 
                     const std::string &boolean = "and", 
                     bool isnot = false) {
     // If the val is a query builder instance we will assume the developer wants to 
     // look for any vals that exists within this given query. So we will add the
     // query accordingly so that this query is properly executed when it is run.
     return where_in_existing_query(column, query, boolean, isnot);
   };

   //Add a "where in" clause to the query.
   Builder &where_in(const std::string &column, 
                     closure_t callback, 
                     const std::string &boolean = "and", 
                     bool isnot = false) {
     // If the val of the where in clause is actually a Closure, we will assume that
     // the developer is using a full sub-select for this "in" statement, and will
     // execute those Closures, then we can re-construct the entire sub-selects.
     return where_insub(column, callback, boolean, isnot);
   };

   //Add an "or where in" clause to the query.
   Builder &or_where_in(const std::string &column, 
                        const variable_array_t &vals) {
     return where_in(column, vals, "or");
   };

   //Add an "or where in" clause to the query.
   Builder &or_where_in(const std::string &column, 
                        Builder &query) {
     return where_in(column, query, "or");
   };

   //Add an "or where in" clause to the query.
   Builder &or_where_in(const std::string &column, 
                        closure_t callback) {
     return where_in(column, callback, "or");
   };

   //Add a "where not in" clause to the query.
   Builder &where_notin(const std::string &column,
                        const variable_array_t &vals,
                        const std::string &boolean = "and") {
     return where_in(column, vals, boolean, true);
   };

   //Add a "where not in" clause to the query.
   Builder &where_notin(const std::string &column,
                        Builder &query,
                        const std::string &boolean = "and") {
     return where_in(column, query, boolean, true);
   };

   //Add a "where not in" clause to the query.
   Builder &where_notin(const std::string &column,
                        closure_t callback,
                        const std::string &boolean = "and") {
     return where_in(column, callback, boolean, true); 
   };

   //Add an "or where not in" clause to the query.
   Builder &or_where_notin(const std::string &column,
                           const variable_array_t &vals) {
     return where_notin(column, vals, "or");
   };
   
   //Add an "or where not in" clause to the query.
   Builder &or_where_notin(const std::string &column,
                           Builder &query) {
     return where_notin(column, query, "or");
   };
   
   //Add an "or where not in" clause to the query.
   Builder &or_where_notin(const std::string &column,
                           closure_t callback) {
     return where_notin(column, callback, "or");
   };

   //Add a "where null" clause to the query.
   Builder &where_null(const std::string &column, 
                       const std::string &boolean = "and", 
                       bool isnot = false);

   //Add an "or where null" clause to the query.
   Builder &or_where_null(const std::string &column) {
     return where_null(column, "or");
   };

   //Add a "where not null" clause to the query.
   Builder &where_notnull(const std::string &column, 
                          const std::string &boolean = "and") {
     return where_null(column, boolean, true);
   };

   //Add a where between statement to the query.
   Builder &where_between(const std::string &column,
                          const variable_array_t &vals,
                          const std::string &boolean = "and",
                          bool isnot = false);

   //Add an or where between statement to the query.
   Builder &or_where_between(const std::string &column,
                             const variable_array_t &vals) {
     return where_between(column, vals, "or");
   };

   //Add a where not between statement to the query.
   Builder &where_notbetween(const std::string &column,
                             const variable_array_t &vals,
                             const std::string &boolean = "and") {
     return where_between(column, vals, boolean, true);
   };

   //Add an or where not between statement to the query.
   Builder &or_where_notbetween(const std::string &column,
                                const variable_array_t &vals) {
     return where_notbetween(column, vals, "or");
   };

   //Add an "or where not null" clause to the query.
   Builder &or_where_notnull(const std::string &column) {
     return where_notnull(column, "or");
   };

   //Add a "where date" statement to the query.
   Builder &where_date(const std::string &column,
                       const std::string &oper,
                       const variable_t &val = "",
                       const std::string &boolean = "and");

   //Add an "or where date" statement to the query.
   Builder &or_where_date(const std::string &column,
                          const std::string &oper,
                          const std::string &val) {
     return where_date(column, oper, val, "or");
   };

   //Add a "where time" statement to the query.
   Builder &where_time(const std::string &column, 
                       const std::string &oper, 
                       const variable_t &val, 
                       const std::string &boolean = "and") {
     return add_date_based_where("time", column, oper, val, boolean);
   };

   //Add an "or where time" statement to the query.
   Builder &or_where_time(const std::string &column, 
                          const std::string &oper, 
                          const variable_t &val) {
     return where_time(column, oper, val, "or");
   };

   //Add a "where day" statement to the query.
   Builder &where_day(const std::string &column, 
                      const std::string &oper, 
                      const variable_t &val = "", 
                      const std::string &boolean = "and");

   //Add a "where month" statement to the query.
   Builder &where_month(const std::string &column, 
                        const std::string &oper, 
                        const variable_t &val = "", 
                        const std::string &boolean = "and");

   //Add a "where year" statement to the query.
   Builder &where_year(const std::string &column, 
                       const std::string &oper, 
                       const variable_t &val = "", 
                       const std::string &boolean = "and");

   //Add a nested where statement to the query.
   Builder &where_nested(closure_t callback, const std::string &boolean = "and") {
     auto query = for_nested_where();
     callback(query);
     return add_nested_where_query(query, boolean);
   };

   //Create a new query instance for nested where condition.
   // * This function will create heap pointer with new_query function.
   Builder *for_nested_where() {
     auto query = new_query();
     query->from(from_);
     return query;
   };

   //Add another query builder as a nested where to the query builder.
   Builder &add_nested_where_query(Builder *query,
                                   const std::string &boolean = "and");

   //Add an exists clause to the query.
   Builder &where_exists(closure_t callback, 
                         const std::string &boolean = "and", 
                         bool isnot = false);

   //Add an or exists clause to the query.
   Builder &or_where_exists(closure_t callback, bool isnot = false) {
     return where_exists(callback, "or", isnot);
   };

   //Add a where not exists clause to the query.
   Builder &where_not_exists(closure_t callback, 
                             const std::string &boolean = "and") {
     return where_exists(callback, boolean, true);
   };

   //Add a or where not exists clause to the query.
   Builder &or_where_not_exists(closure_t callback) {
     return or_where_exists(callback, true);
   };

   //Add an exists clause to the query.
   // * The query pointer with delete by this function.
   Builder &add_where_exists_query(Builder *query, 
                                   const std::string &boolean = "and",
                                   bool isnot = false);

   //Handles dynamic "where" clauses to the query.
   // * Not complete.
   Builder &dynamic_where(const std::string &method,
                          const std::string &parameters);

   //Add a "group by" clause to the query.
   Builder &group_by(const std::vector<std::string> &groups);

   //Add a "having" clause to the query.
   Builder &having(const std::string &column, 
                   const std::string &oper = "", 
                   const variable_t &val = "", 
                   const std::string &boolean = "and");

   //Add a "or having" clause to the query.
   Builder &or_having(const std::string &column, 
                      const std::string &oper = "", 
                      const variable_t &val = "") {
     return having(column, oper, val, "or");
   };

   //Add a raw having clause to the query.
   Builder &having_raw(const std::string &sql,
                       const variable_array_t &bindings,
                       const std::string &boolean = "and");

   //Add a raw or having clause to the query.
   Builder &or_having_raw(const std::string &sql,
                          const variable_array_t &bindings) {
     return having_raw(sql, bindings, "or");
   };

   //Add an "order by" clause to the query.
   Builder &order_by(const std::string &column, 
                     const std::string &direction = "asc");

   //Add a descending "order by" clause to the query.
   Builder &order_bydesc(const std::string &column) {
     return order_by(column, "desc");
   };
  
   //Add an "order by" clause for a timestamp to the query.
   Builder &latest(const std::string &column = "created_at") {
     return order_by(column, "desc");
   };

   //Add an "order by" clause for a timestamp to the query.
   Builder &oldest(const std::string &column = "created_at") {
     return order_by(column, "asc");
   };

   //Put the query's results in random order.
   Builder &in_random_order(const std::string &seed = "");

   //Add a raw "order by" clause to the query.
   Builder &order_byraw(const std::string &sql,
                        const variable_array_t &bindings = {});

   //Alias to set the "offset" val of the query.
   Builder &skip(int32_t val) {
     return offset(val);
   };

   //Set the "offset" val of the query.
   Builder &offset(int32_t val);

   //Alias to set the "limit" val of the query.
   Builder &take(int32_t val) {
     return limit(val);
   };

   //Set the "limit" val of the query.
   Builder &limit(int32_t val);

   //Set the limit and offset for a given page.
   Builder &for_page(int32_t page, int32_t perpage = 15) {
     return skip((page - 1) * perpage).take(perpage);
   };

   //Constrain the query to the next "page" of results after a given ID.
   Builder &for_page_afterid(int32_t perpage, 
                             int32_t lastid, 
                             const std::string &column = "id") {
     std::vector<variable_set_t> orders;
     remove_existing_orders_for(column, orders);
     orders_ = orders;
     return where(column, ">", lastid).order_by(column, "desc").take(perpage);
   };
   
   //Add a union statement to the query.
   Builder &_union(closure_t callback, bool all = false);

   //Add a union statement to the query.
   // * This function will delete the query pointer.
   Builder &_union(Builder *query, bool all = false);

   //Add a union all statement to the query.
   // * This function will delete the query pointer.
   Builder &union_all(Builder *query) {
     return _union(query, true);
   };

    //Lock the selected rows in the table.
   Builder &lock(const variable_t &val = true) {
     lock_ = val;
     return *this;
   };

   //Lock the selected rows in the table for updating.
   Builder &lock_forupdate() {
     return lock(true);
   };

   //Share lock the selected rows in the table.
   Builder &shared_lock() {
     return lock(false);
   };

   //Get the SQL representation of the query.
   std::string to_sql();

   //Execute a query for a single record by ID.
   variable_array_t find(int32_t id, 
                         const std::vector<std::string> columns = {"*"});

   //Get a single column's val from the first result of a query.
   variable_t value(const std::string &column);

   //Execute the query as a "select" statement.
   db_fetch_array_t get(const std::vector<std::string> &columns = {"*"});

   //Run the query as a "select" statement against the connection.
   db_fetch_array_t run_select();

   //* Get a generator for the given query.
   void cursor();

   //Chunk the results of a query by comparing numeric IDs.
   bool chunk_byid(int32_t count, 
                   closure_t callback, 
                   const std::string &column = "id", 
                   const std::string &alias = "");

   //Throw an exception if the query doesn't have an order_by clause.
   void enforce_order_by();

   //Get an array with the vals of a given column.
   void pluck(const std::string &column, 
              db_fetch_array_t &result, 
              const std::string &key = "");

   //Concatenate vals of a given column as a string.
   const std::string implode(
       const std::string &column, const std::string &glue) const;

   //Determine if any rows exist for the current query.
   bool exists();

   //Retrieve the "count" result of the query.
   int32_t count(const std::string &columns = "*") {
     return aggregate("count", {columns}).get<int32_t>();
   };

   //Retrieve the "count" result of the query.
   int32_t count(const std::vector<std::string> &columns) {
     return aggregate("count", columns).get<int32_t>();
   };

   //Retrieve the minimum val of a given column.
   variable_t _min(const std::string &column) {
     return aggregate("_min", {column});
   };

   //Retrieve the sum of the vals of a given column.
   variable_t _max(const std::string &column) {
     return aggregate("_max", {column});
   };

   //Retrieve the sum of the vals of a given column.
   variable_t sum(const std::string &column) {
     return aggregate("sum", {column});
   };

   //Retrieve the average of the vals of a given column.
   variable_t avg(const std::string &column) {
     return aggregate("avg", {column});
   };

   //Alias for the "avg" method.
   variable_t average(const std::string &column) {
     return avg(column);
   };

   //Execute an aggregate function on the database.
   variable_t aggregate(const std::string &function, 
                        const std::vector<std::string> &columns = {"*"});

   //Execute a numeric aggregate function on the database.
   variable_t numeric_aggregate(
       const std::string &function, 
       const std::vector<std::string> &columns = {"*"});

   //Insert a new record into the database.
   bool insert(variable_set_t &vals) {
     std::vector<variable_set_t> _vals{ {vals} };
     return insert(_vals);
   };

   //Insert a new record into the database.
   bool insert(std::vector<variable_set_t> &vals);

   //Insert a new record and get the val of the primary key.
   int32_t insert_getid(const variable_array_t &vals, 
                        const std::string &sequence = "");

   //Update a record in the database.
   int32_t update(variable_set_t &vals);

   //Insert or update a record matching the attributes, and fill it with vals.
   bool update_or_insert(variable_set_t &attributes,
                         variable_set_t &vals);

   //Increment a column's val by a given amount.
   int32_t increment(const std::string &column, 
                     int32_t amount = 1, 
                     const variable_array_t &extra = {});

   //Decrement a column's val by a given amount.
   int32_t decrement(const std::string &column, 
                     int32_t amount = 1, 
                     const variable_set_t &extra = {});

   //Delete a record from the database.
   int32_t deleted(const variable_t &id = "");

   //Run a truncate statement on the table.
   void truncate();

   //Get a new instance of the query builder.
   //* This function will return the new object from heap, you need ensure safe
   // delete it.
   virtual Builder *new_query() {
     return new Builder(connection_, grammar_);
   };

   //* Create a raw database expression.
   variable_t raw(const variable_t &val);

   //Get the current query val bindings in a flattened array.
   variable_array_t get_bindings();
 
   //Get the raw array of bindings.
   db_query_bindings_t *get_raw_bindings() {
     return &bindings_;
   };

   //Set the bindings on the query builder.
   Builder &set_bindings(variable_array_t &bindings, 
                         const std::string &type = "where");

   //Add a binding to the query.
   Builder &add_bindings(const variable_array_t &vals, 
                        const std::string &type = "where");

   //Add a binding to the query.
   Builder &add_binding(const variable_t &val, 
                        const std::string &type = "where");

   //Merge an array of bindings into our bindings.
   Builder &merge_bindings(Builder &query);

   //Remove all of the expressions from a list of bindings.
   variable_array_t clean_bindings_expression(const variable_array_t &bindings);

   //Get the database connection instance.
   ConnectionInterface *get_connection() {
     return connection_;
   };

   //Get the query grammar instance.
   grammars::Grammar *get_grammar() {
     return grammar_;
   };

   //Clean the members by the variable name without "_".
   Builder &clean(const std::vector<std::string> &except) {
     for (const std::string &name : except)
       clean(name);
     return *this;
   };

   //Clean the member by the variable name without "_".
   Builder &clean(const std::string &except);

   //Clean the given bindings.
   Builder &clean_bindings(const std::vector<std::string> &except);

 public:
   //Add an "on" clause to the join.
   virtual Builder &on(const std::string &, 
                      const std::string &, 
                      const std::string &, 
                      const std::string &) {
     return *this;
   };

   //Add an "on" clause to the join.
   virtual Builder &on(closure_t, 
                       const std::string &, 
                       const std::string &, 
                       const std::string &) {
      return *this;
   };

   //Add an "or on" clause to the join.
   virtual Builder &or_on(const std::string &,
                  const std::string &,
                  const std::string &) {
     return *this;
   };

   //Add an "or on" clause to the join.
   virtual Builder &or_on(closure_t) {
     return *this;
   };

 protected:

   //Prepare the val and operator for a where clause.
   variable_array_t prepare_value_and_operator(
       const std::string &val, 
       const std::string &oper, 
       bool use_default = false);

   //Determine if the given operator and val combination is legal.
   // - Prevents using Null vals with invalid operators.
   bool invalid_operator_and_value(const std::string &oper, 
                                   const variable_t &val);

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
                                 const variable_t &val,
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
                                   std::vector<variable_set_t> &result);

   //Remove the column aliases since they will break count queries.
   void without_select_aliases(std::vector<std::string> &columns);

   //Strip off the table name or alias from a column identifier.
   const std::string strip_table_for_pluck(const std::string &column);

   //Set the aggregate property without running the query.
   Builder &set_aggregate(const std::string &function, 
                          const std::vector<std::string> &columns);
};

} //namespace query

} //namespace pf_db

#endif //PF_DB_QUERY_BUILDER_H_
