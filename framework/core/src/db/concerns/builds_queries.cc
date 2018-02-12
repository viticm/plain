#include "pf/db/query/builder.h"
#include "pf/db/concerns/builds_queries.h"

using namespace pf_db::concerns;

#define tobuilder(o) ((pf_db::query::Builder *)o)

//Apply the callback's query changes if the given "value" is true.
pf_db::query::Builder &BuildsQueries::when(
    const variable_t &value, 
    value_closure_t callback,
    value_closure_t def) {
  auto builder = tobuilder(this);
  if (value != "" && value.get<bool>()) {
    callback(builder, value);
  } else if (!is_null(def)) {
    def(builder, value);
  }
  return *builder;
}

//Apply the callback's query changes if the given "value" is true. 
pf_db::query::Builder &BuildsQueries::unless(
    const variable_t &value, 
    value_closure_t callback, 
    value_closure_t def) {
  auto builder = tobuilder(this);
  if (value != "" && value.get<bool>()) {
    callback(builder, value);
  } else if (!is_null(def)) {
    def(builder, value);
  }
  return *builder;
}
