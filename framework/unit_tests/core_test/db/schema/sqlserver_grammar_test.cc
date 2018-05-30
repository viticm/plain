#include "gtest/gtest.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/sqlserver_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db;
using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaSqlserverGrammar : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //Normal.
     auto connection = new pf_db::Connection(engine->get_db());
     unique_move(pf_db::Connection, connection, connection_);

     auto mysql_grammar = new grammars::SqlserverGrammar;
     unique_move(grammars::Grammar, mysql_grammar, sqlserver_grammar_);

     auto blueprint = new Blueprint("");
     unique_move(Blueprint, blueprint, blueprint_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
     blueprint_->clear();
     sqlserver_grammar_->clear();
   }

   virtual void TearDown() {
   }

 protected:
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> sqlserver_grammar_;
   static std::unique_ptr<Blueprint> blueprint_;

};

std::unique_ptr<grammars::Grammar> DBSchemaSqlserverGrammar::sqlserver_grammar_{nullptr};
std::unique_ptr<pf_db::Connection> DBSchemaSqlserverGrammar::connection_{nullptr};
std::unique_ptr<Blueprint> DBSchemaSqlserverGrammar::blueprint_{nullptr};

static void assertEquals(
    const std::vector<std::string> &a, 
    const std::vector<std::string> &b, 
    int32_t line = -1) {
  if (line != -1)
    std::cout << "assertEquals: " << line << std::endl;
  ASSERT_TRUE(a.size() == b.size());
  for (size_t i = 0; i < a.size(); ++i)
    ASSERT_STREQ(a[i].c_str(), b[i].c_str());
}

static void print_r(const std::vector<std::string> &array) {
  for (auto &str : array)
    std::cout << "str: " << str << std::endl;
}

TEST_F(DBSchemaSqlserverGrammar, testBasicCreateTable) {
  blueprint_->set_table("users");
  blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create table \"users\" (\"id\" int identity primary key not \
null, \"email\" nvarchar(255) not null)", 
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->increments("id");
  blueprint_->string("email");

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"id\" int identity primary key not \
null, \"email\" nvarchar(255) not null", 
               statements1[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");

  sqlserver_grammar_->set_table_prefix("prefix_");

  auto statements2 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements2.size());

  ASSERT_STREQ("create table \"prefix_users\" (\"id\" int identity primary key not \
null, \"email\" nvarchar(255) not null)", 
               statements2[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropTable) {
  blueprint_->set_table("users");
  blueprint_->drop();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table \"users\"", 
               statements[0].c_str());
  
  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->drop();

  sqlserver_grammar_->set_table_prefix("prefix_");

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("drop table \"prefix_users\"", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropTableIfExists) {
  blueprint_->set_table("users");
  blueprint_->drop_if_exists();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("if exists (select * from INFORMATION_SCHEMA.TABLES where \
TABLE_NAME = 'users') drop table \"users\"", 
               statements[0].c_str());
  
  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->drop_if_exists();

  sqlserver_grammar_->set_table_prefix("prefix_");

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("if exists (select * from INFORMATION_SCHEMA.TABLES where \
TABLE_NAME = 'prefix_users') drop table \"prefix_users\"", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropColumn) {
  blueprint_->set_table("users");
  blueprint_->drop_column({"foo", "bar"});

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop column \"foo\", \"bar\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropPrimary) {
  blueprint_->set_table("users");
  blueprint_->drop_primary("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop constraint \"foo\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropUnique) {
  blueprint_->set_table("users");
  blueprint_->drop_unique("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop index \"foo\" on \"users\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropIndex) {
  blueprint_->set_table("users");
  blueprint_->drop_index("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop index \"foo\" on \"users\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropForeign) {
  blueprint_->set_table("users");
  blueprint_->drop_foreign("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop constraint \"foo\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropTimestamps) {
  blueprint_->set_table("users");
  blueprint_->drop_timestamps();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop column \"created_at\", \"updated_at\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testDropTimestampsTz) {
  blueprint_->set_table("users");
  blueprint_->drop_timestamps_tz();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop column \"created_at\", \"updated_at\"",
               statements[0].c_str());
}
