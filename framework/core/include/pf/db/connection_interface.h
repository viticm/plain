/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id connection_interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/11/30 14:46
 * @uses The database connection interface.
*/
#ifndef PF_DB_CONNECTION_INTERFACE_H_
#define PF_DB_CONNECTION_INTERFACE_H_

#include "pf/db/config.h"
#include "pf/db/query/builder.h"

namespace pf_db {

class PF_API ConnectionInterface {

 public:
   ConnectionInterface() {}
   virtual ~ConnectionInterface() {}

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_t = pf_basic::type::variable_t;
   using callback_t = std::function<void(ConnectionInterface *)>;

 public:

   //Begin a fluent query against a database table.
   virtual query::Builder table(const std::string &name) = 0;

   //Get a new raw query expression.
   virtual void raw(variable_t &value) = 0;

   //Run a select statement and return a single result.
   virtual void select_one(
       const std::string &str, const variable_array_t &bindings = {}) = 0;

   //Run a select statement against the database.
   virtual bool insert(
       const std::string &str, const variable_array_t &bindings = {}) = 0;

   //Run an update statement against the database.
   virtual int32_t update(
       const std::string &str, const variable_array_t &bindings = {}) = 0;

   //Run a delete statement against the database.
   virtual int32_t deleted(
       const std::string &str, const variable_array_t &bindings = {}) = 0;

   //Execute an SQL statement and return the boolean result.
   virtual bool statement(
       const std::string &str, const variable_array_t &bindings = {}) = 0;

   //Run an SQL statement and get the number of rows affected.
   virtual int32_t affecting_statement(
       const std::string &str, const variable_array_t &bindings = {}) = 0;

   //Run a raw, unprepared query against the PDO connection.
   virtual bool unprepared(const std::string &str) = 0;

   //Prepare the query bindings for execution.
   virtual void prepare_bindings(variable_array_t &bindings) = 0;

   //Execute a Closure within a transaction.
   virtual void transaction(callback_t callback, int8_t attempts = 1) = 0;

   //Start a new database transaction.
   virtual void begin_transaction() = 0;

   //Commit the active database transaction.
   virtual void commit() = 0;

   //Rollback the active database transaction.
   virtual void rollback() = 0;

   //Get the number of active transactions.
   virtual int32_t transaction_level() const = 0;

   //Execute the given callback in "dry run" mode.
   virtual void pretend(callback_t callback) = 0;

};

}; //namespace pf_db

#endif //PF_DB_CONNECTION_INTERFACE_H_
