/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id sqlserver_grammar.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/05 15:12
 * @uses your description
*/
#ifndef PF_DB_QUERY_GRAMMARS_SQLSERVER_GRAMMAR_H_
#define PF_DB_QUERY_GRAMMARS_SQLSERVER_GRAMMAR_H_

#include "pf/db/query/grammars/grammar.h"

namespace pf_db {

namespace query {

namespace grammars {

class PF_API SqlserverGrammar : public grammars::Grammar {

 public:
   SqlserverGrammar();
   virtual ~SqlserverGrammar();

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using Builder = pf_db::query::Builder;

 public:

   //Compile a select query into SQL.
   virtual std::string compile_select(Builder &query);

   //Compile an insert statement into SQL.
   virtual std::string compile_insert(
       Builder &query, const variable_array_t &values);

   //Compile a truncate table statement into SQL.
   virtual variable_set_t compile_truncate(Builder &query);

   //Compile the random statement into SQL.
   virtual std::string compile_random(const std::string &seed);

   //Compile a where exists clause.
   virtual std::string where_exists(Builder &query, const variable_set_t &where);

   //Compile an insert and get ID statement into SQL.
   virtual std::string compile_insert_getid(
       Builder &query, 
       const variable_array_t &values, 
       const std::string &sequence);

   //Compile an update statement into SQL.
   virtual std::string compile_update(
       Builder &query, const variable_array_t &values);

   //Prepare the bindings for an update statement.
   virtual variable_set_t prepare_bindings_forupdate(
       const variable_set_t &bindings, const variable_array_t &values);

   //Compile a delete statement into SQL.
   virtual std::string compile_delete(Builder &query);

 protected:
   
   //Compile the "select *" portion of the query.
   virtual std::string compile_columns(
       Builder &query, const std::vector<std::string> &columns);

   //Compile the "from" portion of the query.
   virtual std::string compile_from(Builder &query, const std::string &table);

   //Compile a "where date" clause.
   virtual std::string where_date(Builder &query, const variable_set_t &where);

   //Compile the "limit" portions of the query.
   virtual std::string compile_limit(Builder &query, int32_t limit);

   //Compile a single union statement.
   virtual std::string compile_union(const variable_set_t &unions);

   //Compile the "union" queries attached to the main query.
   virtual std::string compile_unions(Builder &query);

   //Compile the lock into SQL.
   virtual std::string compile_lock(Builder &query, const std::string &value);

   //Concatenate an array of segments, removing empties.
   virtual std::string concatenate(const std::vector<std::string> &segments);

   //Remove the leading boolean from a statement.
   virtual std::string remove_leading_boolean(const std::string &value);

 protected:

   //Create a full ANSI offset clause for the query.
   std::string compile_ansi_offset(
       Builder &query, const variable_set_t &components);

   //Compile the over statement for a table expression.
   std::string compile_over(const std::string &orderings);

   //Compile a common table expression for a query.
   std::string compile_table_expression(const std::string &sql, Builder &query);

   //Compile the limit / offset row constraint for a query.
   std::string compile_row_constraint(Builder &query);

   //

};

}; //namespace grammars

}; //namespace query

}; //namespace pf_db

#endif //PF_DB_QUERY_GRAMMARS_SQLSERVER_GRAMMAR_H_
