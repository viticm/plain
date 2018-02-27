/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id sqlite_grammar.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/05 14:58
 * @uses your description
*/
#ifndef PF_DB_QUERY_GRAMMARS_SQLITE_GRAMMAR_H_
#define PF_DB_QUERY_GRAMMARS_SQLITE_GRAMMAR_H_

#include "pf/db/query/grammars/grammar.h"

namespace pf_db {

namespace query {

namespace grammars {

class PF_API SqliteGrammar : public grammars::Grammar {

 public:
   SqliteGrammar();
   virtual ~SqliteGrammar();

 public:

   //Compile a select query into SQL.
   virtual std::string compile_select(Builder &query);

   //Compile an insert statement into SQL.
   virtual std::string compile_insert(
       Builder &query, std::vector<variable_set_t> &values);

   //Compile a truncate table statement into SQL.
   virtual variable_set_t compile_truncate(Builder &query);

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;
   using Builder = pf_db::query::Builder;

 protected:
   
   //Compile a single union statement.
   virtual std::string compile_union(db_query_array_t &_union);

   //Compile a "where day" clause.
   virtual std::string where_day(Builder &query, db_query_array_t &where) {
     return date_based_where("%d", query, where);
   };

   //Compile a "where month" clause.
   virtual std::string where_month(Builder &query, db_query_array_t &where) {
     return date_based_where("%m", query, where);
   };

   //Compile a "where year" clause.
   virtual std::string where_year(Builder &query, db_query_array_t &where) {
     return date_based_where("%Y", query, where);
   };

   //Compile a "where date" clause.
   virtual std::string where_date(Builder &query, db_query_array_t &where) {
     return date_based_where("%Y-%m-%d", query, where);
   };

   //Compile a date based where clause.
   virtual std::string date_based_where(
       const std::string &type, Builder &query, db_query_array_t &where);

};

} //namespace grammars

} //namespace query

} //namespace pf_db

#endif //PF_DB_QUERY_GRAMMARS_SQLITE_GRAMMAR_H_
