/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id builds_queries.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/12/26 16:26
 * @uses your description
*/
#ifndef PF_DB_CONCERNS_BUILDS_QUERIES_H_
#define PF_DB_CONCERNS_BUILDS_QUERIES_H_

#include "pf/db/concerns/config.h"

namespace pf_db {

namespace concerns {

class PF_API BuildsQueries {

 public:
   BuildsQueries(query::Builder *query) : query_{query} {}
   virtual ~BuildsQueries() {};

 public:
   using variable_t = pf_basic::type::variable_t;
   using variable_array_t = pf_basic::type::variable_array_t;
   using check_closure_t = 
     std::function<bool (const variable_t &, const variable_t &)>;
   using value_closure_t = 
     std::function<variable_t (query::Builder *, const variable_t &)>;

 public:

   //Chunk the results of the query.
   bool chunk(int32_t count, check_closure_t callback);

   //Execute a callback over each item while chunking.
   bool each(check_closure_t callback, int32_t count = 1000);

   //Execute the query and get the first result.
   variable_array_t first(const std::vector<std::string> &columns = {"*"});

   //Apply the callback's query changes if the given "value" is true. 
   query::Builder *unless(const variable_t &value, 
                          value_closure_t callback, 
                          value_closure_t def = nullptr);

 protected:

   //The query builder pointer.
   query::Builder *query_;

};

}; //namespace concerns

}; //namespace pf_db

#endif //PF_DB_CONCERNS_BUILDS_QUERIES_H_
