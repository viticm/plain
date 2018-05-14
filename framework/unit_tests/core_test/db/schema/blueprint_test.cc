#include "gtest/gtest.h"
#include "pf/engine/kernel.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/mysql_grammar.h"
#include "pf/db/schema/grammars/postgres_grammar.h"
#include "pf/db/schema/grammars/sqlite_grammar.h"
#include "pf/db/schema/grammars/sqlserver_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaBlueprint : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //Normal.
     auto connection = new pf_db::Connection(engine->get_db());
     unique_move(pf_db::Connection, connection, connection_);

     auto mysql_grammar = new grammars::MysqlGrammar;
     unique_move(grammars::Grammar, mysql_grammar, mysql_grammar_);

     auto postgres_grammar = new grammars::PostgresGrammar;
     unique_move(grammars::Grammar, postgres_grammar, postgres_grammar_);

     auto sqlserver_grammar = new grammars::SqlserverGrammar;
     unique_move(grammars::Grammar, sqlserver_grammar, sqlserver_grammar_);

     auto sqlite_grammar = new grammars::SqliteGrammar;
     unique_move(grammars::Grammar, sqlite_grammar, sqlite_grammar_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
   }

   virtual void TearDown() {
   }

 protected:
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> mysql_grammar_;
   static std::unique_ptr<grammars::Grammar> postgres_grammar_;
   static std::unique_ptr<grammars::Grammar> sqlserver_grammar_;
   static std::unique_ptr<grammars::Grammar> sqlite_grammar_;

};

std::unique_ptr<grammars::Grammar> DBSchemaBlueprint::mysql_grammar_{nullptr};
std::unique_ptr<grammars::Grammar> DBSchemaBlueprint::postgres_grammar_{nullptr};
std::unique_ptr<grammars::Grammar> DBSchemaBlueprint::sqlserver_grammar_{nullptr};
std::unique_ptr<grammars::Grammar> DBSchemaBlueprint::sqlite_grammar_{nullptr};

void assertEquals(
    const std::vector<std::string> &a, 
    const std::vector<std::string> &b, 
    int32_t line = -1) {
  if (line != -1)
    std::cout << "assertEquals: " << line << std::endl;
  ASSERT_TRUE(a.size() == b.size());
  for (size_t i = 0; i < a.size(); ++i)
    ASSERT_STREQ(a[i].c_str(), b[i].c_str());
}

std::unique_ptr<pf_db::Connection> DBSchemaBlueprint::connection_{nullptr};
using fluent_t = pf_db::db_schema_fluent_t;

TEST_F(DBSchemaBlueprint, testIndexDefaultNames) {
  Blueprint blueprint("users");
  blueprint.unique({"foo", "bar"});
  auto commands = blueprint.get_commands();
  ASSERT_STREQ("users_foo_bar_unique", commands[0]["index"].c_str());

  Blueprint blueprint1("users");
  blueprint1.index({"foo"});
  auto commands1 = blueprint1.get_commands();
  ASSERT_STREQ("users_foo_index", commands1[0]["index"].c_str());
}

TEST_F(DBSchemaBlueprint, testDropIndexDefaultNames) {
  auto create_blueprint = []{
    auto blueprint = new Blueprint("users", [](Blueprint *table){
      table->timestamp("created")["use_current"] = true;
    });
    return blueprint;
  };
  std::unique_ptr<Blueprint> blueprint1(create_blueprint());
  assertEquals({"alter table `users` add `created` timestamp "
"default CURRENT_TIMESTAMP not null"},
               blueprint1->to_sql(connection_.get(), mysql_grammar_.get()),
               __LINE__);

  std::unique_ptr<Blueprint> blueprint2(create_blueprint());
  assertEquals({"alter table \"users\" add column \"created\" timestamp(0) "
"without time zone default CURRENT_TIMESTAMP(0) not null"},
               blueprint2->to_sql(connection_.get(), postgres_grammar_.get()),
               __LINE__);

  std::unique_ptr<Blueprint> blueprint3(create_blueprint());
  assertEquals({"alter table \"users\" add column \"created\" "
"datetime default CURRENT_TIMESTAMP not null"},
               blueprint3->to_sql(connection_.get(), sqlite_grammar_.get()),
               __LINE__);

  std::unique_ptr<Blueprint> blueprint4(create_blueprint());
  assertEquals({"alter table \"users\" add \"created\" "
"datetime default CURRENT_TIMESTAMP not null"},
               blueprint4->to_sql(connection_.get(), sqlserver_grammar_.get()),
               __LINE__);
}
