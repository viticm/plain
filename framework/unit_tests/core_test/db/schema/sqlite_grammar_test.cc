#include "gtest/gtest.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/sqlite_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db;
using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaSqliteGrammar : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //Normal.
     auto connection = new pf_db::Connection(engine->get_db());
     unique_move(pf_db::Connection, connection, connection_);

     auto sqlite_grammar = new grammars::SqliteGrammar;
     unique_move(grammars::Grammar, sqlite_grammar, sqlite_grammar_);

     auto blueprint = new Blueprint("");
     unique_move(Blueprint, blueprint, blueprint_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
     blueprint_->clear();
     sqlite_grammar_->clear();
   }

   virtual void TearDown() {
   }

 protected:
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> sqlite_grammar_;
   static std::unique_ptr<Blueprint> blueprint_;

};

std::unique_ptr<grammars::Grammar> DBSchemaSqliteGrammar::sqlite_grammar_{nullptr};
std::unique_ptr<pf_db::Connection> DBSchemaSqliteGrammar::connection_{nullptr};
std::unique_ptr<Blueprint> DBSchemaSqliteGrammar::blueprint_{nullptr};

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

/**
static void print_r(const std::vector<std::string> &array) {
  for (auto &str : array)
    std::cout << "str: " << str << std::endl;
}
**/

TEST_F(DBSchemaSqliteGrammar, testBasicCreateTable) {
  blueprint_->set_table("users");
  blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create table \"users\" (\"id\" integer not null primary key \
autoincrement, \"email\" varchar not null)", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();
  
  blueprint_->set_table("users");
  blueprint_->increments("id");
  blueprint_->string("email");
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get());
  
  std::vector<std::string> expected{
    "alter table \"users\" add column \"id\" integer not null primary key autoincrement; \
alter table \"users\" add column \"email\" varchar not null",
  };
  assertEquals(expected, statements1, __LINE__);
}

TEST_F(DBSchemaSqliteGrammar, testDropTable) {
  blueprint_->set_table("users");
  blueprint_->drop();
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table \"users\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testDropTableIfExists) {
  blueprint_->set_table("users");
  blueprint_->drop_if_exists();
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table if exists \"users\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testDropUnique) {
  blueprint_->set_table("users");
  blueprint_->drop_unique("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop index \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testDropIndex) {
  blueprint_->set_table("users");
  blueprint_->drop_index("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop index \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testRenameTable) {
  blueprint_->set_table("users");
  blueprint_->rename("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" rename to \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingPrimaryKey) {
  blueprint_->set_table("users");
  blueprint_->create();
  auto &column = blueprint_->string("foo");
  column["primary"] = true;
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create table \"users\" (\"foo\" varchar not null, primary key (\"foo\"))", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingForeignKey) {
  blueprint_->set_table("users");
  blueprint_->create();

  auto &column = blueprint_->string("foo");
  column["primary"] = true;
  blueprint_->string("order_id");
  auto &column1 = blueprint_->foreign({"order_id"});
  column1.references.emplace_back("id");
  column1["on"] = "orders";

  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create table \"users\" (\"foo\" varchar not null, \"order_id\" \
varchar not null, foreign key(\"order_id\") references \"orders\"(\"id\"), \
primary key (\"foo\"))", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingUniqueKey) {
  blueprint_->set_table("users");
  blueprint_->unique({"foo"}, "bar");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create unique index \"bar\" on \"users\" (\"foo\")", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingIndex) {
  blueprint_->set_table("users");
  blueprint_->index({"foo", "bar"}, "baz");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create index \"baz\" on \"users\" (\"foo\", \"bar\")", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->increments("id");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" integer not null \
primary key autoincrement", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingSmallIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->small_increments("id");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" integer not null \
primary key autoincrement", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingMediumIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->medium_increments("id");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" integer not null \
primary key autoincrement", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingBigIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->big_increments("id");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" integer not null \
primary key autoincrement", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingString) {
  blueprint_->set_table("users");
  blueprint_->string("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar not null", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->string("foo", 100);
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar not null", 
               statements1[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  auto &column = blueprint_->string("foo", 100).nullable();
  column["default"] = "bar";
  auto statements2 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements2.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar null \
default 'bar'", 
               statements2[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingText) {
  blueprint_->set_table("users");
  blueprint_->text("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" text not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingBigInteger) {
  blueprint_->set_table("users");
  blueprint_->big_integer("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->big_integer("foo", true);
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null \
primary key autoincrement", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingInteger) {
  blueprint_->set_table("users");
  blueprint_->integer("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->integer("foo", true);
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null \
primary key autoincrement", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingMediumInteger) {
  blueprint_->set_table("users");
  blueprint_->medium_integer("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->medium_integer("foo", true);
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null \
primary key autoincrement", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTinyInteger) {
  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo", true);
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null \
primary key autoincrement", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingSmallInteger) {
  blueprint_->set_table("users");
  blueprint_->small_integer("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  sqlite_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->small_integer("foo", true);
  auto statements1 = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null \
primary key autoincrement", 
               statements1[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingFloat) {
  blueprint_->set_table("users");
  blueprint_->_float("foo", 5, 2);
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" float not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingDouble) {
  blueprint_->set_table("users");
  blueprint_->_double("foo", 15, 8);
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" float not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingDecimal) {
  blueprint_->set_table("users");
  blueprint_->decimal("foo", 5, 2);
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" numeric not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingBoolean) {
  blueprint_->set_table("users");
  blueprint_->boolean("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" tinyint(1) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingEnum) {
  blueprint_->set_table("users");
  blueprint_->_enum("foo", {"bar", "baz"});
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingJson) {
  blueprint_->set_table("users");
  blueprint_->json("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" text not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingJsonb) {
  blueprint_->set_table("users");
  blueprint_->jsonb("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" text not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingDate) {
  blueprint_->set_table("users");
  blueprint_->date("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" date not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingDateTime) {
  blueprint_->set_table("users");
  blueprint_->date_time("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" datetime not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingDateTimeTz) {
  blueprint_->set_table("users");
  blueprint_->date_time_tz("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" datetime not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTime) {
  blueprint_->set_table("users");
  blueprint_->time("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" time not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTimeTz) {
  blueprint_->set_table("users");
  blueprint_->time_tz("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" time not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTimeStamp) {
  blueprint_->set_table("users");
  blueprint_->timestamp("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" datetime not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTimeStampTz) {
  blueprint_->set_table("users");
  blueprint_->timestamp_tz("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 
  
  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" datetime not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTimeStamps) {
  blueprint_->set_table("users");
  blueprint_->timestamps();
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"created_at\" datetime null; \
alter table \"users\" add column \"updated_at\" datetime null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingTimeStampsTz) {
  blueprint_->set_table("users");
  blueprint_->timestamps_tz();
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"created_at\" datetime null; \
alter table \"users\" add column \"updated_at\" datetime null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingRememberToken) {
  blueprint_->set_table("users");
  blueprint_->remember_token();
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"remember_token\" varchar null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingBinary) {
  blueprint_->set_table("users");
  blueprint_->binary("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" blob not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingUuid) {
  blueprint_->set_table("users");
  blueprint_->uuid("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingIpAddress) {
  blueprint_->set_table("users");
  blueprint_->ip_address("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaSqliteGrammar, testAddingMacAddress) {
  blueprint_->set_table("users");
  blueprint_->mac_address("foo");
  auto statements = blueprint_->to_sql(connection_.get(), sqlite_grammar_.get()); 

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar not null", 
               statements[0].c_str());
}
