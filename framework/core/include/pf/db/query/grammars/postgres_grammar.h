/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id postgres_grammar.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/05 14:28
 * @uses your description
*/
#ifndef PF_DB_QUERY_GRAMMARS_POSTGRES_GRAMMAR_H_
#define PF_DB_QUERY_GRAMMARS_POSTGRES_GRAMMAR_H_

#include "pf/db/query/grammars/grammar.h"

namespace pf_db {

namespace query {

namespace grammars {

class PF_API PostgresGrammar : public grammars::Grammar {

 public:
   PostgresGrammar();
   virtual ~PostgresGrammar();

 public:

   //Compile an insert statement into SQL.
   virtual std::string compile_insert(
       Builder &query, const variable_array_t &values);

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

   //Compile a truncate table statement into SQL.
   virtual variable_set_t compile_truncate(Builder &query);

public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using Builder = pf_db::query::Builder;

 protected:

   //Compile a "where date" clause.
   virtual std::string where_date(Builder &query, const variable_set_t &where);

   //Compile a date based where clause.
   virtual std::string date_based_where(
       const std::string &type, Builder &query, const variable_set_t &where);

   //Compile the lock into SQL.
   virtual std::string compile_lock(Builder &query, const std::string &value);

 protected:

   //Compile the columns for the update statement.
   std::string compile_update_columns(const variable_array_t &values);

   //Compile the "from" clause for an update with a join.
   std::string compile_update_from(Builder &query);

   //Compile the additional where clauses for updates with joins.
   std::string compile_update_wheres(Builder &query);

   //Compile the "join" clause where clauses for an update.
   std::string compile_update_join_wheres(Builder &query);

   //Compile a delete query that uses joins.
   std::string compile_delete_with_joins(
       Builder &query, const std::string &table, const variable_set_t &where);

   //Wrap a single string in keyword identifiers.
   std::string wrap_value(const std::string &value);

    //Wrap the given JSON selector.
    std::string wrap_json_selector(const std::string &value);

    //Wrap the attributes of the give JSON path.
    void wrap_json_path_attributes(std::vector<std::string> path);

};

} //namespace grammars

} //namespace query

} //namespace pf_db

#endif //PF_DB_QUERY_GRAMMARS_POSTGRES_GRAMMAR_H_
