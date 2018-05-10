#include "pf/db/schema/grammars/change_column.h"

using namespace pf_db::schema::grammars;

//Compile a change column command into a series of SQL statements.
std::vector<std::string> ChangeColumn::compile(
    Grammar *, 
    Blueprint *, 
    fluent_t &, 
    ConnectionInterface *) {
  return {"",};
}

//Get the Doctrine table difference for the given changes.
bool ChangeColumn::get_changed_diff(
   Grammar *, Blueprint *, void *) {
  return false;
}

//Get a copy of the given Doctrine table after making the column changes.
bool ChangeColumn::get_table_with_column_changes(Blueprint *, void *) {
  return false;
}

//Get the Doctrine column instance for a column change.
bool ChangeColumn::get_doctrine_column(void *, const std::string &) {
  return false;
}
