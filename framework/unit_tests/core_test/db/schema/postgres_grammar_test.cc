#include "gtest/gtest.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/postgres_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db;
using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaPostgresGrammar : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //Normal.
     auto connection = new pf_db::Connection(engine->get_db());
     unique_move(pf_db::Connection, connection, connection_);

     auto postgres_grammar = new grammars::PostgresGrammar;
     unique_move(grammars::Grammar, postgres_grammar, postgres_grammar_);

     auto blueprint = new Blueprint("");
     unique_move(Blueprint, blueprint, blueprint_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
     blueprint_->clear();
     postgres_grammar_->clear();
   }

   virtual void TearDown() {
   }

 protected:
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> postgres_grammar_;
   static std::unique_ptr<Blueprint> blueprint_;

};

std::unique_ptr<grammars::Grammar> DBSchemaPostgresGrammar::postgres_grammar_{nullptr};
std::unique_ptr<pf_db::Connection> DBSchemaPostgresGrammar::connection_{nullptr};
std::unique_ptr<Blueprint> DBSchemaPostgresGrammar::blueprint_{nullptr};

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

TEST_F(DBSchemaPostgresGrammar, testBasicCreateTable) {
  blueprint_->set_table("users");
  blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");
  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create table \"users\" (\"id\" serial primary key not null, \
\"email\" varchar(255) not null)", statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropTable) {
  blueprint_->set_table("users");
  blueprint_->drop();
  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table \"users\"", statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropTableIfExists) {
  blueprint_->set_table("users");
  blueprint_->drop_if_exists();
  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop table if exists \"users\"", statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropColumn) {
  blueprint_->set_table("users");
  blueprint_->drop_column({"foo", "bar"});

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop column \"foo\", drop column \"bar\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropPrimary) {
  blueprint_->set_table("users");
  blueprint_->drop_primary();

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop constraint \"users_pkey\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropUnique) {
  blueprint_->set_table("users");
  blueprint_->drop_unique("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop constraint \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropIndex) {
  blueprint_->set_table("users");
  blueprint_->drop_index("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("drop index \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropForeign) {
  blueprint_->set_table("users");
  blueprint_->drop_foreign("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop constraint \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropTimestamps) {
  blueprint_->set_table("users");
  blueprint_->drop_timestamps();

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop column \"created_at\", \
drop column \"updated_at\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testDropTimestampsTz) {
  blueprint_->set_table("users");
  blueprint_->drop_timestamps_tz();

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" drop column \"created_at\", \
drop column \"updated_at\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testRenameTable) {
  blueprint_->set_table("users");
  blueprint_->rename("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" rename to \"foo\"", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingPrimaryKey) {
  blueprint_->set_table("users");
  blueprint_->primary({"foo"});

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add primary key (\"foo\")", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingUniqueKey) {
  blueprint_->set_table("users");
  blueprint_->unique({"foo"}, "bar");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add constraint \"bar\" unique (\"foo\")", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingIndex) {
  blueprint_->set_table("users");
  blueprint_->index({"foo", "bar"}, "baz");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create index \"baz\" on \"users\" (\"foo\", \"bar\")", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingIndexWithAlgorithm) {
  blueprint_->set_table("users");
  blueprint_->index({"foo", "bar"}, "baz", "hash");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("create index \"baz\" on \"users\" using hash (\"foo\", \"bar\")", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" serial primary key not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingSmallIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->small_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" smallserial primary \
key not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingMediumIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->medium_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" serial primary key not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingBigIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->big_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"id\" bigserial \
primary key not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingString) {
  blueprint_->set_table("users");
  blueprint_->string("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar(255) not null", 
               statements[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();
  blueprint_->set_table("users");
  blueprint_->string("foo", 100);

  auto statements1 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar(100) not null", 
               statements1[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();
  blueprint_->set_table("users");
  auto &column = blueprint_->string("foo", 100).nullable();
  column["default"] = "bar";

  auto statements2 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements2.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar(100) null \
default 'bar'", 
               statements2[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingText) {
  blueprint_->set_table("users");
  blueprint_->text("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" text not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingBigInteger) {
  blueprint_->set_table("users");
  blueprint_->big_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" bigint not null", 
               statements[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->big_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" bigserial primary \
key not null", 
               statements1[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingInteger) {
  blueprint_->set_table("users");
  blueprint_->integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" serial primary \
key not null", 
               statements1[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingMediumInteger) {
  blueprint_->set_table("users");
  blueprint_->medium_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" integer not null", 
               statements[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->medium_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" serial primary \
key not null", 
               statements1[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingTinyInteger) {
  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" smallint not null", 
               statements[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" smallserial primary \
key not null", 
               statements1[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingSmallInteger) {
  blueprint_->set_table("users");
  blueprint_->small_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" smallint not null", 
               statements[0].c_str());

  blueprint_->clear();
  postgres_grammar_->clear();

  blueprint_->set_table("users");
  blueprint_->small_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" smallserial primary \
key not null", 
               statements1[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingFloat) {
  blueprint_->set_table("users");
  blueprint_->_float("foo", 5, 2);

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" double precision not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingDouble) {
  blueprint_->set_table("users");
  blueprint_->_double("foo", 15, 8);

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" double precision not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingDecimal) {
  blueprint_->set_table("users");
  blueprint_->decimal("foo", 5, 2);

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" decimal(5, 2) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingBoolean) {
  blueprint_->set_table("users");
  blueprint_->boolean("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" boolean not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingEnum) {
  blueprint_->set_table("users");
  blueprint_->_enum("foo", {"bar", "baz"});

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" varchar(255) check \
(\"foo\" in ('bar', 'baz')) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingDate) {
  blueprint_->set_table("users");
  blueprint_->date("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" date not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingJson) {
  blueprint_->set_table("users");
  blueprint_->json("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" json not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingJsonb) {
  blueprint_->set_table("users");
  blueprint_->jsonb("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" jsonb not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingDatetime) {
  blueprint_->set_table("users");
  blueprint_->date_time("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" timestamp(0) without \
time zone not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingDatetimeTz) {
  blueprint_->set_table("users");
  blueprint_->date_time_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" timestamp(0) with \
time zone not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingTime) {
  blueprint_->set_table("users");
  blueprint_->time("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" time(0) without time \
zone not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingTimeTz) {
  blueprint_->set_table("users");
  blueprint_->time_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" time(0) with time \
zone not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingTimeStamp) {
  blueprint_->set_table("users");
  blueprint_->timestamp("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" timestamp(0) without \
time zone not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingTimeStampTz) {
  blueprint_->set_table("users");
  blueprint_->timestamp_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"foo\" timestamp(0) with \
time zone not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaPostgresGrammar, testAddingTimeStamps) {
  blueprint_->set_table("users");
  blueprint_->timestamps();

  auto statements = blueprint_->to_sql(connection_.get(), postgres_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table \"users\" add column \"created_at\" timestamp(0) \
without time zone null, add column \"updated_at\" timestamp(0) without time \
zone null", 
               statements[0].c_str());
}
