#include "pf/basic/string.h"
#include "pf/db/interface.h"
#include "pf/basic/stringstream.h"
#include "pf/basic/io.tcc"
#include "pf/db/query.h"

namespace pf_db {

Query::Query()
  : tablename_{0},
  env_{nullptr},
  sql_{""},
  isready_{false} {
}
   
Query::~Query() {
  //do nothing
}

bool Query::init(Interface *env) {
  if (nullptr == env) return false;
  env_ = env;
  isready_ = true;
  return true;
}

void Query::set_tablename(const std::string &tablename) {
  pf_basic::string::safecopy(tablename_, tablename.c_str(), sizeof(tablename_));
}

bool Query::query() {
  if (!isready_ || is_null(env_)) return false;
  bool result = env_->query(sql_);
  return result;
}

bool Query::select(const std::string &string) {
  if (!isready_) return false;
  sql_.clear(); sql_ += "select ";
  sql_ += string;
  return true;
}

bool Query::_delete(const std::string &string) {
  if (!isready_) return false;
  sql_.clear(); sql_ += "delete ";
  sql_ += string;
  return true;
}
   
bool Query::where(const std::string &string) {
  if (!isready_) return false;
  sql_ += " where "; sql_ += string;
  return true;
}

bool Query::select(const pf_basic::type::variable_array_t &values) {
  if (!isready_) return false;
  uint32_t count = static_cast<uint32_t>(values.size());
  if (0 == count) return false;
  sql_.clear();
  sql_ += "select";
  for (uint32_t i = 0; i < count; ++i) {
    sql_ += values[i].c_str();
    if (i != count - 1) sql_ += ",";
  }
  return true;
}
   
bool Query::_delete() {
  if (!isready_) return false;
  sql_.clear(); sql_ += "delete";
  return true;
}

bool Query::insert(const pf_basic::type::variable_array_t &keys, 
                   const pf_basic::type::variable_array_t &values) {
  if (!isready_) return false;
  if (0 == strlen(tablename_)) return false;
  uint32_t keycount = static_cast<uint32_t>(keys.size());
  uint32_t valuecount = static_cast<uint32_t>(values.size());
  if (0 == keycount || keycount != valuecount) return false;
  sql_.clear();
  uint32_t i;
  sql_ += "insert into "; sql_ += tablename_;
  sql_ += " (";
  for (i = 0; i < keycount; ++i) {
    sql_ += keys[i].c_str();
    if (i != keycount - 1) sql_ += ", ";
  }
  sql_ += ") values (";
  for (i = 0; i < keycount; ++i) { 
    auto isstring = pf_basic::type::kVariableTypeString == values[i].type;
    if (isstring) sql_ += "'";
    sql_ += values[i].c_str();
    if (isstring) sql_ += "'";
    if (i != keycount - 1) sql_ += ", ";
  }
  sql_ += ")";
  return true;
}

bool Query::update(const pf_basic::type::variable_array_t &keys, 
                   const pf_basic::type::variable_array_t &values) {
  if (!isready_) return false;
  if (0 == strlen(tablename_)) return false;
  uint32_t keycount = static_cast<uint32_t>(keys.size());
  uint32_t valuecount = static_cast<uint32_t>(values.size());
  if (0 == keycount || keycount != valuecount) return false;
  sql_.clear();
  sql_ += "update "; sql_ += tablename_; sql_ += " set ";
  uint32_t i;
  for (i = 0; i < keycount; ++i) { 
    auto isstring = pf_basic::type::kVariableTypeString == values[i].type;
    sql_ += keys[i].c_str(); sql_ += "=";
    if (isstring) sql_ += "'";
    sql_ += values[i].c_str();
    if (isstring) sql_ += "'";
    if (i != keycount - 1) sql_ += ", ";
  }
  return true;
}

bool Query::update(const pf_basic::type::variable_array_t &keys, 
                   const pf_basic::type::variable_array_t &values,
                   dbenv_t dbenv) {
  using namespace pf_basic;
  if (!isready_) return false;
  if (0 == strlen(tablename_)) return false;
  bool result = false;
  auto keycount = keys.size();
  auto valuecount = values.size();
  if (0 == keycount || valuecount % keycount != 0) return false;
  sql_.clear();
  switch (dbenv) {
    case kDBEnvMysql: {
      size_t i{0};
      sql_ += "replace into "; sql_ += tablename_; sql_ += " (";
      for (i = 0; i < keycount; ++i) { 
        sql_ += keys[i].c_str();
        if (i != keycount - 1) sql_ += ",";
      }
      sql_ += ") values ";
      size_t row = valuecount / keycount;
      for (i = 0; i < row; ++i) {
        sql_ += "(";
        for (size_t j = 0; j < keycount; ++j) {
          const type::variable_t &value = values[i * keycount + j];
          auto isstring = value.type == type::kVariableTypeString;
          if (isstring) sql_ += "'";
          sql_ += value.c_str();
          if (isstring) sql_ += "'";
          if (j != keycount - 1) sql_ += ",";
        }
        sql_ += ")";
        if (i == row - 1) 
          sql_ += ";";
        else
          sql_ += ",";
      }
      result = true;
      break;
    }
    default:
      break;
  }
  return result;
}
   
bool Query::from() {
  if (!isready_) return false;
  if (0 == strlen(tablename_)) return false;
  sql_ += " from ";
  sql_ += tablename_;
  return true;
}
 
bool Query::where(const pf_basic::type::variable_t &key, 
                  const pf_basic::type::variable_t &value, 
                  const std::string &operator_str) {
  if (!isready_) return false;
  sql_ += " where ";
  auto isstring = pf_basic::type::kVariableTypeString == value.type;
  sql_ += key.c_str();
  sql_ += operator_str;
  if (isstring) sql_ += "'";
  sql_ += value.c_str();
  if (isstring) sql_ += "'";
  return true;
}

bool Query::_and(const pf_basic::type::variable_t &key, 
                 const pf_basic::type::variable_t &value, 
                 const std::string &operator_str) {
  if (!isready_) return false;
  sql_ += " and ";
  auto isstring = pf_basic::type::kVariableTypeString == value.type;
  sql_ += key.c_str();
  sql_ += operator_str;
  if (isstring) sql_ += "'";
  sql_ += value.c_str();
  if (isstring) sql_ += "'";
  return true;
}

bool Query::_or(const pf_basic::type::variable_t &key, 
                const pf_basic::type::variable_t &value, 
                const std::string &operator_str) {
  if (!isready_) return false;
  sql_ += " or ";
  auto isstring = pf_basic::type::kVariableTypeString == value.type;
  sql_ += key.c_str();
  sql_ += operator_str;
  if (isstring) sql_ += "'";
  sql_ += value.c_str();
  if (isstring) sql_ += "'";
  return true;
}
   
bool Query::limit(int32_t m, int32_t n) {
  if (!isready_) return false;
  pf_basic::type::variable_t varm{m};
  pf_basic::type::variable_t varn{n};
  sql_ += " limit ";
  if (0 == n) {
    sql_ += varm.c_str();
  } else {
    sql_ += varm.c_str();
    sql_ += " , ";
    sql_ += varn.c_str();
  }
  return true;
}

bool Query::fetcharray(db_fetch_array_t &db_fetch_array) {
  using namespace pf_basic::type;
  if (!isready_ || is_null(env_)) return false;
  if (!env_->fetch()) return false;
  int32_t columncount = env_->get_columncount();
  if (columncount <= 0) return false;
  int32_t i = 0;
  //read keys
  for (i = 0; i < columncount; ++i) {
    const char *columnname = env_->get_columnname(i);
    db_fetch_array.keys.push_back(columnname);
  }
  //read values
  do {
    for (i = 0; i < columncount; ++i) {
      variable_t value = env_->get_data(i, "");
      auto columntype = env_->gettype(i);
      if (kDBColumnTypeString == columntype) {
        value.type = kVariableTypeString;
      } else if (kDBColumnTypeNumber == columntype) {
        value.type = kVariableTypeNumber;
      } else {
        value.type = kVariableTypeInt64;
      }
      db_fetch_array.values.push_back(value);
    }
  } while (env_->fetch());
  return true;
}

bool Query::fetch(char *str, size_t size) {
  using namespace pf_basic;
  if (!isready_ || is_null(env_)) return false;
  if (!env_->fetch()) return false;
  int32_t columncount = env_->get_columncount();
  if (columncount <= 0) return false;
  stringstream sstream(str, size);
  sstream.clear();
  sstream << columncount;
  int32_t i = 0;
  //read keys
  for (i = 0; i < columncount; ++i) {
    const char *columnname = env_->get_columnname(i);
    auto columntype = static_cast<int8_t>(env_->gettype(i));
    sstream << columnname;
    sstream << columntype;
  }
  int32_t row = 0;
  auto row_position = sstream.get_position();
  sstream << row;
  //read values
  do {
    for (i = 0; i < columncount; ++i) {
      type::variable_t value = env_->get_data(i, "");
      auto columntype = env_->gettype(i);
      if (kDBColumnTypeString == columntype) {
        sstream << value.c_str();
      } else if (kDBColumnTypeNumber == columntype) {
        sstream << value.get<double>();
      } else {
        sstream << value.get<int64_t>();
      }
      if (0 == (i + 1) % columncount && i != 0) row += 1;
      if (sstream.full()) return false;
    }
  } while (env_->fetch());
  sstream.set_position(row_position);
  sstream << row;
  return true;
}

bool Query::fetch(
    char *columns, size_t columns_size, char *rows, size_t rows_size) {
  using namespace pf_basic;
  if (!isready_ || is_null(env_)) return false;
  if (!env_->fetch()) return false;
  int32_t columncount = env_->get_columncount();
  if (columncount <= 0) return false;
  stringstream scolumns(columns, columns_size);
  scolumns.clear();
  scolumns << columncount;
  int32_t i{0};
  for (i = 0; i < columncount; ++i) {
    const char *columnname = env_->get_columnname(i);
    auto columntype = static_cast<int8_t>(env_->gettype(i));
    scolumns << columnname;
    scolumns << columntype;
  }
  stringstream srows(rows, rows_size);
  srows.clear();
  int32_t row = 0;
  auto row_position = srows.get_position();
  srows << row;
  do {
    for (i = 0; i < columncount; ++i) {
      type::variable_t value = env_->get_data(i, "");
      auto columntype = env_->gettype(i);
      if (kDBColumnTypeString == columntype) {
        srows << value.c_str();
      } else if (kDBColumnTypeNumber == columntype) {
        srows << value.get<double>();
      } else {
        srows << value.get<int64_t>();
      }
      if (0 == (i + 1) % columncount && i != 0) row += 1;
      if (srows.full()) return false;
    }
  } while (env_->fetch());
  srows.set_position(row_position);
  srows << row;
  return true;
}

} //namespace pf_db
