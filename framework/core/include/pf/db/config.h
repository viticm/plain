/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2016/06/19 20:24
 * @uses the defines for db module
 */
#ifndef PF_DB_CONFIG_H_
#define PF_DB_CONFIG_H_

#include "pf/basic/config.h"
#include "pf/basic/type/variable.h"
#include "pf/basic/hashmap/template.h"

#define SQL_LENGTH_MAX (1024*400)
#define DB_CONNECTION_NAME_LENGTH 128 
#define DB_DBNAME_LENGTH 128
#define DB_USER_NAME_LENGTH 32
#define DB_PASSWORD_LENGTH 32
#define DB_TABLENAME_LENGTH 64
#define DB_PREFIX_LENGTH 32
#define DB_COLUMN_COUNT_MAX 64

#define DB_MODULENAME "db"

/* The database environment invalid id. */
#define DB_EID_INVALID (-1)

/* The database expression variable type. */
#define DB_EXPRESSION_TYPE (100)

/* The database expression variable type(the original is a real string). *not use */
#define DB_EXPRESSION_TYPE_S (101)

/* The database binding keys. */
#define DB_BINDING_KEYS {"select", "join", "where", "having", "order", "union"} 

typedef enum {
  kDBColumnTypeString = 0,
  kDBColumnTypeNumber,
  kDBColumnTypeInteger,
} db_columntype_t; //字段类型

/* db fetch and cache { */
typedef std::map<std::string, int8_t> db_keys_t;
/* } db fetch and cache */

typedef struct PF_API db_fetch_array_struct db_fetch_array_t;
struct db_fetch_array_struct {

  //Database cloumn names.
  pf_basic::type::variable_array_t keys;

  //Result values.
  pf_basic::type::variable_array_t values;

  db_fetch_array_struct();
  db_fetch_array_struct(const db_fetch_array_t &object);
  db_fetch_array_struct(const db_fetch_array_t *object);
  db_fetch_array_t &operator = (const db_fetch_array_t &object);
  db_fetch_array_t *operator = (const db_fetch_array_t *object);
  pf_basic::type::variable_t *get(int32_t row, const char *key);
  pf_basic::type::variable_t *get(int32_t row, int32_t column);
  uint32_t size() const;
  void clear();
};

//The connector type.
/**
typedef enum {
  kDBConnectorTypeODBC = 0, //方便以后扩展
} dbconnector_type_t;
**/

//The db environment type.
typedef enum {
  kDBEnvNull = 0,
  kDBEnvODBC = 1,
  kDBEnvMysql = 2,
  kDBEnvPostgres = 3,
  kDBEnvSqlite = 4,
  kDBEnvSqlserver = 5,
} dbenv_t;

namespace pf_db {

class Query;
class Interface;
class Factory;
class ConnectionInterface;
class Connection;

struct config_struct {
  std::string name; //connection or db name.
  std::string username;
  std::string password;
  int8_t type;
};

using config_t = config_struct;
using eid_t = int16_t; //Environment.

namespace concerns {

class BuildsQueries;

}

namespace query {

class Builder;
class JoinClause;

namespace grammars {

class Grammar;
class MysqlGrammar;
class PostgresGrammar;
class SqliteGrammar;
class SqlserverGrammar;

}

}

namespace schema {

class Blueprint;
class Builder;
class MySqlBuilder;
class PostgresBuilder;

namespace grammars {

class ChangeColumn;
class RenameColumn;
class Grammar;
class MysqlGrammar;
class PostgresGrammar;
class SqliteGrammar;
class SqlserverGrammar;

}

}

//The query array, has an variable set and query pointer.
typedef struct PF_API db_query_array_struct db_query_array_t;
struct db_query_array_struct {

  //The variable set.
  pf_basic::type::variable_set_t items;

  //The query builder pointer.
  std::shared_ptr<query::Builder> query;

  //The values of query(always it is empty).
  pf_basic::type::variable_set_t values;

  pf_basic::type::variable_t operator [] (const std::string &key) {
    return items[key];
  };

};

//The fluent for db schema garmmar.
typedef struct PF_API db_schema_fluent_struct db_schema_fluent_t;
struct db_schema_fluent_struct {

  //The variable set.
  pf_basic::type::variable_set_t items;

  //The columns.
  std::vector<std::string> columns;

  //The references.
  std::vector<std::string> references;

  //The allowed.
  std::vector<std::string> allowed;

  pf_basic::type::variable_t &operator [] (const std::string &key) {
    return items[key];
  };

  //The nullable attribute set.
  db_schema_fluent_struct &nullable() {
    items["nullable"] = true;
    return *this;
  }

};

using db_query_bindings_t = 
  std::map<std::string, pf_basic::type::variable_array_t>;

//Get the raw variable.
inline pf_basic::type::variable_t raw(
    const pf_basic::type::variable_t &value) {
  using namespace pf_basic::type;
  auto r = value;
  r.type = static_cast<var_t>(DB_EXPRESSION_TYPE);
  return r;
}

inline pf_basic::type::variable_t null() {
  pf_basic::type::variable_t r;
  return r;
}

inline bool is_expression(const pf_basic::type::variable_t &value) {
  return DB_EXPRESSION_TYPE == value.type;
}

} //namespace pf_db


//Auto environment creator.
auto_envcreator(db, pf_db::Interface)

#endif //PF_DB_CONFIG_H_
