/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id connection.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/01/03 10:19
 * @uses your description
*/
#ifndef PF_DB_CONNECTION_H_
#define PF_DB_CONNECTION_H_

#include "pf/db/config.h"
#include "pf/db/connection_interface.h"

namespace pf_db {

class PF_API Connection : public ConnectionInterface {

 public:
   using variable_array_t = pf_basic::type::variable_array_t;
   using variable_set_t = pf_basic::type::variable_set_t;
   using variable_t = pf_basic::type::variable_t;

 public:

   //The construct function.
   Connection(Interface *env, 
              const std::string &database = "", 
              const std::string &table_prefix = "", 
              const variable_set_t &config = {});

   //The destruct function.
   virtual ~Connection();

 public:

   //Set the query grammar to the default implementation.
   void use_default_query_grammar();

 public:

   //Run a select statement against the database.
   virtual db_fetch_array_t select(const std::string &query, 
                                   const variable_array_t &bindings);

   //Begin a fluent query against a database table.
   virtual query::Builder table(const std::string &name);

   //Get a new raw query expression.
   virtual variable_t raw(const variable_t &value);

   //Run a select statement and return a single result.
   virtual db_fetch_array_t select_one(
       const std::string &str, const variable_array_t &bindings);

   //Run a select statement against the database.
   virtual bool insert(
       const std::string &str, const variable_array_t &bindings);

   //Run an update statement against the database.
   virtual int32_t update(
       const std::string &str, const variable_array_t &bindings);

   //Run a delete statement against the database.
   virtual int32_t deleted(
       const std::string &str, const variable_array_t &bindings);

   //Execute an SQL statement and return the boolean result.
   virtual bool statement(
       const std::string &str, const variable_array_t &bindings);

   //Run an SQL statement and get the number of rows affected.
   virtual int32_t affecting_statement(
       const std::string &str, const variable_array_t &bindings);

   //Run a raw, unprepared query against the PDO connection.
   virtual bool unprepared(const std::string &str);

   //Prepare the query bindings for execution.
   virtual void prepare_bindings(db_query_bindings_t &bindings);

   //Execute a Closure within a transaction.
   virtual void transaction(closure_t callback, int8_t attempts);

   //Start a new database transaction.
   virtual void begin_transaction();

   //Commit the active database transaction.
   virtual void commit();

   //Rollback the active database transaction.
   virtual void rollback();

   //Get the number of active transactions.
   virtual int32_t transaction_level() const;

   //Execute the given callback in "dry run" mode.
   virtual void pretend(closure_t callback);

   //Get the query grammar used by the connection.
   virtual query::grammars::Grammar *get_query_grammar();

   //Get the schema grammar used by the connection.
   virtual query::grammars::Grammar *get_schema_grammar();

 public:

   // Determine if the connection in a "dry run".
   bool pretending() const {
     return true == pretending_;
   }

 protected:

   // The environment pointer.
   Interface *env_;

   // The name of the connected database.
   std::string database_;

   // The table prefix for the connection.
   std::string table_prefix_;

   // The database connection configuration options.
   variable_set_t config_;

   // The query grammar pointer.
   std::unique_ptr<query::grammars::Grammar> query_grammar_;

   // Indicates if the connection is in a "dry run".
   bool pretending_;

   // The number of active transactions.
   int32_t transactions_;

 protected:

   // Run a SQL statement and log its execution context.
   template <class F>
   auto run(const std::string &query, const variable_array_t &bindings, F&& f)
   -> typename std::result_of<
   F(const std::string &, const variable_array_t &)>::type {
     env_->check_db_connect(true);
     auto result = run_query_callback(query, bindings, f);
     return result;
   };

   // Run a SQL statement.
   template <class F>
   auto run_query_callback(
       const std::string &query, const variable_array_t &bindings, F&& f)
   -> typename std::result_of<
   F(const std::string &, const variable_array_t &)>::type {
     return f(query, bindings);
   };

};

}; //namespace pf_db

#endif //PF_DB_CONNECTION_H_
