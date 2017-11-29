#include "pf/basic/string.h"
#include "pf/sys/assert.h"
#include "pf/db/define.h"

db_fetch_array_struct::db_fetch_array_struct() {
  clear();
}

pf_basic::type::variable_t *db_fetch_array_struct::get(
    int32_t row, int32_t column) {
  if (INDEX_INVALID == column || 0 == row) return nullptr;
  uint32_t columncount = static_cast<uint32_t>(keys.size());
  if (0 == columncount) return nullptr;
  uint32_t valuecount = static_cast<uint32_t>(values.size());
  uint32_t totalrow = valuecount / columncount;
  if (row > static_cast<int32_t>(totalrow)) return nullptr;
  pf_basic::type::variable_t *result = &values[(row - 1) * columncount];
  pf_basic::type::variable_t *_result = &result[column];
  return _result;
}

pf_basic::type::variable_t *db_fetch_array_struct::get(
    int32_t row, const char *key) {
  int32_t keyindex = INDEX_INVALID;
  for (uint8_t i = 0; i < keys.size(); ++i) {
    if (0 == strcmp(keys[i].c_str(), key)) {
      keyindex = i;
    }
  }
  pf_basic::type::variable_t *result = get(row, keyindex);
  return result;
}

uint32_t db_fetch_array_struct::size() const {
  uint32_t columncount = static_cast<uint32_t>(keys.size());
  if (0 == columncount) return 0;
  uint32_t valuecount = static_cast<uint32_t>(values.size());
  uint32_t totalrow = valuecount / columncount;
  return totalrow;
}

db_fetch_array_struct::db_fetch_array_struct(const db_fetch_array_t &object) {
  keys = object.keys;
  values = object.values;
}

db_fetch_array_struct::db_fetch_array_struct(const db_fetch_array_t *object) {
  if (object) {
    keys = object->keys;
    values = object->values;
  }
}

db_fetch_array_t &
  db_fetch_array_struct::operator = (const db_fetch_array_t &object) {
  keys = object.keys;
  values = object.values;
  return *this;
}
  
db_fetch_array_t *
  db_fetch_array_struct::operator = (const db_fetch_array_t *object) {
  if (object) {
    keys = object->keys;
    values = object->values;
  }
  return this;
}

void db_fetch_array_struct::clear() {
  values.clear();
}
