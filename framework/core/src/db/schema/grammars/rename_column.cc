#include "pf/db/schema/grammars/rename_column.h"

using namespace pf_db::schema::grammars;

//Compile a change column command into a series of SQL statements.
std::vector<std::string> RenameColumn::compile(
    Grammar *, 
    Blueprint *, 
    fluent_t &, 
    ConnectionInterface *) {
  return {"", };
}
