#include "gtest/gtest.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/mysql_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaMysqlGrammar : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //Normal.
     auto connection = new pf_db::Connection(engine->get_db());
     unique_move(pf_db::Connection, connection, connection_);

     auto mysql_grammar = new grammars::MysqlGrammar;
     unique_move(grammars::Grammar, mysql_grammar, mysql_grammar_);

     auto blueprint = new Blueprint("");
     unique_move(Blueprint, blueprint, blueprint_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
     blueprint_->clear();
     mysql_grammar_->clear();
   }

   virtual void TearDown() {
   }

 protected:
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> mysql_grammar_;
   static std::unique_ptr<Blueprint> blueprint_;

};

std::unique_ptr<grammars::Grammar> DBSchemaMysqlGrammar::mysql_grammar_{nullptr};
std::unique_ptr<pf_db::Connection> DBSchemaMysqlGrammar::connection_{nullptr};
std::unique_ptr<Blueprint> DBSchemaMysqlGrammar::blueprint_{nullptr};

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

TEST_F(DBSchemaMysqlGrammar, testBasicCreateTable) {
  blueprint_->set_table("users");
  //blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `id` int unsigned not null \
auto_increment primary key, add `email` varchar(255) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testEngineCreateTable) {

}

TEST_F(DBSchemaMysqlGrammar, testCharsetCollationCreateTable) {

}

TEST_F(DBSchemaMysqlGrammar, testBasicCreateTableWithPrefix) {
  blueprint_->set_table("users");
  blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");
  mysql_grammar_->set_table_prefix("prefix_");
  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create table `prefix_users` (`id` int unsigned not null \
auto_increment primary key, `email` varchar(255) not null)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropTable) {
  blueprint_->set_table("users");
  blueprint_->drop();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table `users`", statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropTableIfExists) {
  blueprint_->set_table("users");
  blueprint_->drop_if_exists();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table if exists `users`", statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropColumn) {
  blueprint_->set_table("users");
  blueprint_->drop_column({"foo", "bar"});

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` drop `foo`, drop `bar`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropPrimary) {
  blueprint_->set_table("users");
  blueprint_->drop_primary();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` drop primary key", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropUnique) {
  blueprint_->set_table("users");
  blueprint_->drop_unique("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` drop index `foo`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropIndex) {
  blueprint_->set_table("users");
  blueprint_->drop_index("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` drop index `foo`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropForeign) {
  blueprint_->set_table("users");
  blueprint_->drop_foreign("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` drop foreign key `foo`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testDropTimestamps) {
  blueprint_->set_table("users");
  blueprint_->drop_timestamps();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` drop `created_at`, drop `updated_at`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testRenameTable) {
  blueprint_->set_table("users");
  blueprint_->rename("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("rename table `users` to `foo`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingPrimaryKey) {
  blueprint_->set_table("users");
  blueprint_->primary({"foo"}, "bar");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add primary key `bar`(`foo`)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingPrimaryKeyWithAlgorithm) {
  blueprint_->set_table("users");
  blueprint_->primary({"foo"}, "bar", "hash");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add primary key `bar` using hash(`foo`)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingUniqueKey) {
  blueprint_->set_table("users");
  blueprint_->unique({"foo"}, "bar");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add unique `bar`(`foo`)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingIndex) {
  blueprint_->set_table("users");
  blueprint_->index({"foo", "bar"}, "baz");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add index `baz`(`foo`, `bar`)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingIndexWithAlgorithm) {
  blueprint_->set_table("users");
  blueprint_->index({"foo", "bar"}, "baz", "hash");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add index `baz` using hash(`foo`, `bar`)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingForeignKey) {
  blueprint_->set_table("users");
  auto &command = blueprint_->foreign({"foo_id"});
  command.references.emplace_back("id");
  command["on"] = "orders";

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add constraint `users_foo_id_foreign` \
foreign key (`foo_id`) references `orders` (`id`)", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `id` int unsigned not null \
auto_increment primary key", 
               statements[0].c_str());
}
