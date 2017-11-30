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

//The db server type.
typedef enum {
  kDBTypeMysql = 0,
} dbtype_t;

namespace pf_db {

class Query;
class Interface;
class Factory;

struct config_struct {
  std::string name; //connection or db name.
  std::string username;
  std::string password;
  int8_t type;
};

using config_t = config_struct;
using eid_t = int16_t; //Environment.

namespace query {

class Builder;

};

}; //namespace pf_db

#define DB_EID_INVALID (-1)

/* The database expression variable type. */
#define DB_EXPRESSION_TYPE (100)

//Auto environment creator.
auto_envcreator(db, pf_db::Interface)

#endif //PF_DB_CONFIG_H_
