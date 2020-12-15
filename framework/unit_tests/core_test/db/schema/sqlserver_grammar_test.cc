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

/**
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
**/

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

TEST_F(DBSchemaSqlserverGrammar, testRenameTable) {
  blueprint_->set_table("users");
  blueprint_->rename("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("sp_rename \"users\", \"foo\"",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingPrimaryKey) {
  blueprint_->set_table("users");
  blueprint_->primary({"foo"}, "bar");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add constraint \"bar\" primary key (\"foo\")",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingUniqueKey) {
  blueprint_->set_table("users");
  blueprint_->unique({"foo"}, "bar");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create unique index \"bar\" on \"users\" (\"foo\")",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingIndex) {
  blueprint_->set_table("users");
  blueprint_->index({"foo", "bar"}, "baz");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create index \"baz\" on \"users\" (\"foo\", \"bar\")",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"id\" int identity primary key not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingSmallIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->small_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"id\" smallint identity primary key not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingMediumIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->medium_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"id\" int identity primary key not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingBigIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->big_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"id\" bigint identity primary key not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingString) {
  blueprint_->set_table("users");
  blueprint_->string("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(255) not null",
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->string("foo", 100);

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(100) not null",
               statements1[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  auto &column = blueprint_->string("foo", 100).nullable();
  column["default"] = "bar";

  auto statements2 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements2.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(100) null default 'bar'",
               statements2[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingText) {
  blueprint_->set_table("users");
  blueprint_->text("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(max) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingBigInteger) {
  blueprint_->set_table("users");
  blueprint_->big_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" bigint not null",
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->big_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" bigint identity primary key not null",
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingInteger) {
  blueprint_->set_table("users");
  blueprint_->integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" int not null",
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" int identity primary key not null",
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingMediumInteger) {
  blueprint_->set_table("users");
  blueprint_->medium_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" int not null",
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->medium_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" int identity primary key not null",
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTinyInteger) {
  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" tinyint not null",
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" tinyint identity primary key not null",
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingSmallInteger) {
  blueprint_->set_table("users");
  blueprint_->small_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" smallint not null",
               statements[0].c_str());

  blueprint_->clear();
  sqlserver_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->small_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" smallint identity primary key not null",
               statements1[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingFloat) {
  blueprint_->set_table("users");
  blueprint_->_float("foo", 5, 2);

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" float not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingDouble) {
  blueprint_->set_table("users");
  blueprint_->_double("foo", 15, 2);

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" float not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingDecimal) {
  blueprint_->set_table("users");
  blueprint_->decimal("foo", 5, 2);

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" decimal(5, 2) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingBoolean) {
  blueprint_->set_table("users");
  blueprint_->boolean("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" bit not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingEnum) {
  blueprint_->set_table("users");
  blueprint_->_enum("foo", {"bar", "baz"});

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(255) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingJson) {
  blueprint_->set_table("users");
  blueprint_->json("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(max) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingJsonb) {
  blueprint_->set_table("users");
  blueprint_->jsonb("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(max) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingDate) {
  blueprint_->set_table("users");
  blueprint_->date("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" date not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingDateTime) {
  blueprint_->set_table("users");
  blueprint_->date_time("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" datetime not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingDateTimeTz) {
  blueprint_->set_table("users");
  blueprint_->date_time_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" datetimeoffset(0) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTime) {
  blueprint_->set_table("users");
  blueprint_->time("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" time not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTimeTz) {
  blueprint_->set_table("users");
  blueprint_->time_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" time not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTimeStamp) {
  blueprint_->set_table("users");
  blueprint_->timestamp("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" datetime not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTimeStampTz) {
  blueprint_->set_table("users");
  blueprint_->timestamp_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" datetimeoffset(0) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTimeStamps) {
  blueprint_->set_table("users");
  blueprint_->timestamps();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"created_at\" datetime null, \
\"updated_at\" datetime null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingTimeStampsTz) {
  blueprint_->set_table("users");
  blueprint_->timestamps_tz();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"created_at\" datetimeoffset(0) \
null, \"updated_at\" datetimeoffset(0) null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingRememberToken) {
  blueprint_->set_table("users");
  blueprint_->remember_token();

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"remember_token\" nvarchar(100) null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingBinary) {
  blueprint_->set_table("users");
  blueprint_->binary("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" varbinary(max) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingUuid) {
  blueprint_->set_table("users");
  blueprint_->uuid("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" uniqueidentifier not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingIpAddress) {
  blueprint_->set_table("users");
  blueprint_->ip_address("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(45) not null",
               statements[0].c_str());
}

TEST_F(DBSchemaSqlserverGrammar, testAddingMacAddress) {
  blueprint_->set_table("users");
  blueprint_->mac_address("foo");

  auto statements = blueprint_->to_sql(connection_.get(), sqlserver_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add \"foo\" nvarchar(17) not null",
               statements[0].c_str());
}
