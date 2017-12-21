/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id mysql_grammar.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/05 13:50
 * @uses your description
*/
#ifndef PF_DB_QUERY_GRAMMARS_MYSQL_GRAMMAR_H_
#define PF_DB_QUERY_GRAMMARS_MYSQL_GRAMMAR_H_

#include "pf/db/query/grammars/grammar.h"

namespace pf_db {

namespace query {

namespace grammars {

class PF_API MysqlGrammar : public grammars::Grammar {

 public:
   MysqlGrammar();
   virtual ~MysqlGrammar() {}

 public:

   //Compile a select query into SQL.
   virtual std::string compile_select(Builder &query);

   //Compile the random statement into SQL.
   virtual std::string compile_random(const std::string &seed);

   //Compile an update statement into SQL.
   virtual std::string compile_update(
       Builder &query, variable_set_t &values);

   //Prepare the bindings for an update statement.
   virtual variable_array_t prepare_bindings_forupdate(
       variable_set_t &bindings, const variable_array_t &values);

   //Compile a delete statement into SQL.
   virtual std::string compile_delete(Builder &query);

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using Builder = pf_db::query::Builder;

 protected:

   //Compile a single union statement.
   virtual std::string compile_union(db_query_array_t &_union);

   //Compile the lock into SQL.
   virtual std::string compile_lock(Builder &query, const variable_t &value);

 protected:

   //Compile all of the columns for an update statement.
   std::string compile_update_columns(variable_set_t &values);

   //Prepares a JSON column being updated using the JSON_SET function.
   std::string compile_json_update_column(
       const std::string &key, const std::string &value);

   //Compile a delete query that does not use joins.
   std::string compile_delete_without_joins(
       Builder &query, const std::string &table, const std::string &where);

   //Compile a delete query that uses joins.
   std::string compile_delete_with_joins(
       Builder &query, const std::string &table, const std::string &where);
 
    //Wrap a single string in keyword identifiers.
    std::string wrap_value(const std::string &value);

    //Wrap the given JSON selector.
    std::string wrap_json_selector(const std::string &value);

    //Determine if the given string is a JSON selector.
    bool is_json_selector(const std::string &value) const;

};

};

}; //namespace query

}; //namespace pf_db

#endif //PF_DB_QUERY_GRAMMARS_MYSQL_GRAMMAR_H_
