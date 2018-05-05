#include "pf/db/schema/grammars/grammar.h"

using namespace pf_db::schema::grammars;

#define compile_call(name) \
  [this](Blueprint *blueprint, fluent_t &command) { \
     return compile_##name(blueprint, command); \
  }

#define safe_call_compile(name, blueprint, command) \
  (compile_calls_.find(name) != compile_calls_.end() ? \
   compile_calls_[name](blueprint, command) : "")

Grammar::Grammar() {
  compile_calls_ = {
    {"add", compile_call(add) },
    {"foreign", compile_call(foreign) },
    {"primary", compile_call(primary) },
    {"unique", compile_call(unique) },
    {"index", compile_call(index) },
    {"drop", compile_call(drop) },
    {"drop_if_exists", compile_call(drop_if_exists) },
    {"drop_column", compile_call(drop_column) },
    {"drop_primary", compile_call(drop_primary) },
    {"drop_unique", compile_call(drop_unique) },
    {"drop_index", compile_call(drop_index) },
    {"drop_foreign", compile_call(drop_foreign) },
  };
}

//The function call compile.
std::vector<std::string> Grammar::call_compile(
    Blueprint *blueprint, 
    fluent_t &command, 
    ConnectionInterface *connection, 
    const std::string &method) {
  std::vector<std::string> r;
  if ("rename_column" == method) {
    std::vector<std::string> rs = 
      compile_rename_column(blueprint, command, connection);
    for (const std::string &item : rs)
      r.emplace_back(item);
  } else if ("change" == method) {
    std::vector<std::string> rs = 
      compile_change(blueprint, command, connection);
    for (const std::string &item : rs)
      r.emplace_back(item);
  } else if ("create" == method) {
    std::string rs = compile_create(blueprint, command, connection);
    if (rs != "") r.emplace_back(rs);
  } else {
    std::string rs = safe_call_compile(method, blueprint, command);
    if (rs != "") r.emplace_back(rs);
  }
  return r;
}
