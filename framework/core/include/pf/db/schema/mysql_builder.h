/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id mysql_builder.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/03/09 15:37
 * @uses your description
*/
#ifndef PF_DB_SCHEMA_MYSQL_BUILDER_H_
#define PF_DB_SCHEMA_MYSQL_BUILDER_H_

#include "pf/db/schema/config.h"
#include "pf/db/schema/builder.h"

namespace pf_db {

namespace schema {

class PF_API MysqlBuilder : public Builder {

 public:

   MysqlBuilder(ConnectionInterface *connection) : Builder(connection) {}
   ~MysqlBuilder() {}

 public:

   //Determine if the given table exists.
   virtual bool has_table(const std::string &table);

   //Get the column listing for a given table.
   virtual std::vector<std::string> get_column_listing(const std::string &table);

};

} //namespace schema

} //namespace pf_db

#endif //PF_DB_SCHEMA_MYSQL_BUILDER_H_
