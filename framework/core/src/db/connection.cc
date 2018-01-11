#include "pf/db/query/grammars/grammar.h"
#include "pf/db/interface.h"
#include "pf/db/connection.h"

using namespace pf_basic::type;
using namespace pf_db;

// The construct function.
Connection::Connection(Interface *env, 
                       const std::string &database, 
                       const std::string &table_prefix, 
                       const variable_set_t &config) {
  env_ = env;

  // First we will setup the default properties. We keep track of the DB
  // name we are connected to since it is needed when some reflective 
  // type commands are run such as checking whether a table exists.
  database_ = database;

  table_prefix_ = table_prefix;

  config_ = config;

  // We need to initialize a query grammar and the query post processors 
  // which are both very important parts of the database abstractions 
  // so we initialize these to their default values while starting.
  use_default_query_grammar();

  transactions_ = 0;
}

// The destruct function.
Connection::~Connection() {

}

// Set the query grammar to the default implementation.
void Connection::use_default_query_grammar() {
  if (!is_null(query_grammar_)) return;
  auto pointer = new query::grammars::Grammar;
  unique_move(query::grammars::Grammar, pointer, query_grammar_);
}

//Run a select statement against the database.
db_fetch_array_t Connection::select(const std::string &query, 
                                    const variable_array_t &bindings) {
  db_fetch_array_t r;
  return run(query, bindings, [this, &r](
        const std::string &_query, const variable_array_t &_bindings){
    if (pretending()) return r;

    // The query sql string and fetch all result.
    if (!env_->query(_query) || !env_->fetch()) return r;

    int32_t columncount = env_->get_columncount();
    if (0 == columncount) return r;

    int32_t i{0};

    // The keys.
    for (i = 0; i < columncount; ++i) { 
      auto columnname = env_->get_columnname(i);
      r.keys.push_back(columnname);
    }

    // The values.
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
        r.values.push_back(value);
      }
    } while (env_->fetch());

    return r;
  });
}

//Begin a fluent query against a database table.
query::Builder *Connection::table(const std::string &name) {
  //query::Builder r(this, get_query_grammar());
  return nullptr;
}

//Get a new raw query expression.
variable_t Connection::raw(const variable_t &value) {
  variable_t r{value};
  r.type = static_cast<var_t>(DB_EXPRESSION_TYPE);
  return value;
}

//Run a select statement and return a single result.
db_fetch_array_t Connection::select_one(
    const std::string &str, const variable_array_t &bindings) {
  auto records = select(str, bindings);
  db_fetch_array_t r;
  if (records.size() > 0) {
    r.keys = records.keys;
    for (size_t i = 0; i < r.keys.size(); ++i)
      r.values.push_back(r.values[i]);
  }
  return r;
}

//Run a select statement against the database.
bool Connection::insert(
    const std::string &str, const variable_array_t &) {
  return env_->query(str);
}

//Run an update statement against the database.
int32_t Connection::update(
    const std::string &str, const variable_array_t &bindings) {
  if (!env_->query(str)) return 0;
  return env_->get_affectcount();
}

//Run a delete statement against the database.
int32_t Connection::deleted(
    const std::string &str, const variable_array_t &bindings) {
  if (!env_->query(str)) return 0;
  return env_->get_affectcount();
}

//Execute an SQL statement and return the boolean result.
bool Connection::statement(
    const std::string &str, const variable_array_t &bindings) {
  return env_->query(str);
}

//Run an SQL statement and get the number of rows affected.
int32_t Connection::affecting_statement(
    const std::string &str, const variable_array_t &bindings) {
  if (!env_->query(str)) return 0;
  return env_->get_affectcount();
}

//Run a raw, unprepared query against the PDO connection.
bool Connection::unprepared(const std::string &str) {
  return run(str, {}, [this](
        const std::string &query, const variable_array_t &bindings){
    if (pretending()) return true;
    return env_->query(query);
  });
}

//Prepare the query bindings for execution.
void Connection::prepare_bindings(db_query_bindings_t &bindings) {

}

//Execute a Closure within a transaction.
void Connection::transaction(closure_t callback, int8_t attempts) {
  //do nothing
}

//Start a new database transaction.
void Connection::begin_transaction() {
}

//Commit the active database transaction.
void Connection::commit() {

}

//Rollback the active database transaction.
void Connection::rollback() {

}

//Get the number of active transactions.
int32_t Connection::transaction_level() const {
  return transactions_;
}

//Execute the given callback in "dry run" mode.
void Connection::pretend(closure_t callback) {

}
