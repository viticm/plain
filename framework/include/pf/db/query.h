/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/pap )
 * $Id query.h
 * @link https://github.com/viticm/pap for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com )
 * @license
 * @user viticm<viticm@126.com>
 * @date 2016/06/19 20:23
 * @uses database query class
 *       cn: 数据库查询器，统一各种语法查询，目前功能较简单
 */
#ifndef PF_DB_QUERY_H_
#define PF_DB_QUERY_H_

#include "pf/db/config.h"

namespace pf_db {

class PF_API Query {

 public:
   Query();
   ~Query();

 public:
   bool init(Interface *manager);
   Interface *getenv() { return env_; };
   void set_tablename(const std::string &tablename);

 public:
   bool select(const std::string &string);
   bool _delete(const std::string &string);
   bool insert(const std::string &string);
   bool update(const std::string &string);
   bool where(const std::string &string);

 public:
   bool select(const pf_basic::type::variable_array_t &values);
   bool _delete();
   bool insert(const pf_basic::type::variable_array_t &keys, 
               const pf_basic::type::variable_array_t &values);
   bool update(const pf_basic::type::variable_array_t &keys, 
               const pf_basic::type::variable_array_t &values);
   bool update(const pf_basic::type::variable_array_t &keys, 
               const pf_basic::type::variable_array_t &values,
               dbtype_t dbtype);

   bool from();
   bool where(const pf_basic::type::variable_t &key, 
              const pf_basic::type::variable_t &value, 
              const std::string &operator_str);
   bool _and(const pf_basic::type::variable_t &key, 
             const pf_basic::type::variable_t &value, 
             const std::string &operator_str);
   bool _or(const pf_basic::type::variable_t &key, 
            const pf_basic::type::variable_t &value, 
            const std::string &operator_str);
   bool limit(int32_t m, int32_t n = 0);

 public:
   bool query();
   bool fetcharray(db_fetch_array_t &db_fetch_array);
   bool fetch(char *str, size_t size);
   bool fetch(char *columns, size_t columns_size, char *rows, size_t rows_size);
   void get_sql(std::string &sql) { sql = sql_; };
   void set_sql(const std::string &sql) { sql_ = sql; }

 private:
   char tablename_[DB_TABLENAME_LENGTH];
   Interface *env_;
   std::string sql_;
   bool isready_;

};

}; //namespace pf_db

#endif //PF_DB_QUERY_H_
