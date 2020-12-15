#include "gtest/gtest.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/mysql_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db;
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

TEST_F(DBSchemaMysqlGrammar, testAddingSmallIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->small_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `id` smallint unsigned not null \
auto_increment primary key", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingBigIncrementingID) {
  blueprint_->set_table("users");
  blueprint_->big_increments("id");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `id` bigint unsigned not null \
auto_increment primary key", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingColumnInTableFirst) {
  blueprint_->set_table("users");
  auto &column = blueprint_->string("name");
  column["first"] = true;

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `name` varchar(255) not null first", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingColumnAfterAnotherColumn) {
  blueprint_->set_table("users");
  auto &column = blueprint_->string("name");
  column["after"] = "foo";

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `name` varchar(255) not null after `foo`", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingGeneratedColumn) {
  blueprint_->set_table("products");
  blueprint_->integer("price");
  auto &column1 = blueprint_->integer("discounted_virtual");
  column1["virtual_as"] = "price - 5";
  auto &column2 = blueprint_->integer("discounted_stored");
  column2["stored_as"] = "price - 5";

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `products` add `price` int not null, \
add `discounted_virtual` int as (price - 5), add `discounted_stored` int \
as (price - 5) stored", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingString) {
  blueprint_->set_table("users");
  blueprint_->string("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(255) not null", 
               statements[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  blueprint_->string("foo", 100);

  auto statements1 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(100) not null", 
               statements1[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  auto &column = blueprint_->string("foo", 100).nullable();
  column["default"] = "bar";

  auto statements2 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements2.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(100) null default 'bar'", 
               statements2[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  auto &column1 = blueprint_->string("foo", 100).nullable();
  column1["default"] = raw("CURRENT TIMESTAMP");

  auto statements3 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements3.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(100) null default \
CURRENT TIMESTAMP", 
               statements3[0].c_str());

}

TEST_F(DBSchemaMysqlGrammar, testAddingText) {
  blueprint_->set_table("users");
  blueprint_->text("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` text not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingBigInteger) {
  blueprint_->set_table("users");
  blueprint_->big_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` bigint not null", 
               statements[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  blueprint_->big_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table `users` add `foo` bigint not null auto_increment \
primary key", 
               statements1[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingInteger) {
  blueprint_->set_table("users");
  blueprint_->integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` int not null", 
               statements[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  blueprint_->integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table `users` add `foo` int not null auto_increment \
primary key", 
               statements1[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingMediumInteger) {
  blueprint_->set_table("users");
  blueprint_->medium_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` mediumint not null", 
               statements[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  blueprint_->medium_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table `users` add `foo` mediumint not null auto_increment \
primary key", 
               statements1[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingSmallInteger) {
  blueprint_->set_table("users");
  blueprint_->small_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` smallint not null", 
               statements[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  blueprint_->small_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table `users` add `foo` smallint not null auto_increment \
primary key", 
               statements1[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTinyInteger) {
  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` tinyint not null", 
               statements[0].c_str());

  blueprint_->clear();
  blueprint_->set_table("users");
  blueprint_->tiny_integer("foo", true);

  auto statements1 = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements1.size());

  ASSERT_STREQ("alter table `users` add `foo` tinyint not null auto_increment \
primary key", 
               statements1[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingFloat) {
  blueprint_->set_table("users");
  blueprint_->_float("foo", 5, 2);

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` double(5, 2) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingDouble) {
  blueprint_->set_table("users");
  blueprint_->_double("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` double not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingDoubleSpecifyingPrecision) {
  blueprint_->set_table("users");
  blueprint_->_double("foo", 15, 8);

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` double(15, 8) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingDecimal) {
  blueprint_->set_table("users");
  blueprint_->decimal("foo", 5, 2);

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` decimal(5, 2) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingBoolean) {
  blueprint_->set_table("users");
  blueprint_->boolean("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` tinyint(1) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingEnum) {
  blueprint_->set_table("users");
  blueprint_->_enum("foo", {"bar", "baz"});

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` enum('bar', 'baz') not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingJson) {
  blueprint_->set_table("users");
  blueprint_->json("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` json not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingJsonb) {
  blueprint_->set_table("users");
  blueprint_->jsonb("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` json not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingDate) {
  blueprint_->set_table("users");
  blueprint_->date("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` date not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingDateTime) {
  blueprint_->set_table("users");
  blueprint_->date_time("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` datetime not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingDateTimeTz) {
  blueprint_->set_table("users");
  blueprint_->date_time_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` datetime not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTime) {
  blueprint_->set_table("users");
  blueprint_->time("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` time not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeTz) {
  blueprint_->set_table("users");
  blueprint_->time_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` time not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeStamp) {
  blueprint_->set_table("users");
  blueprint_->timestamp("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` timestamp not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeStampWithDefault) {
  blueprint_->set_table("users");
  auto &column = blueprint_->timestamp("foo");
  column["default"] = "2018-5-19 15:12:07";

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` timestamp not null default \
'2018-5-19 15:12:07'", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeStampTz) {
  blueprint_->set_table("users");
  blueprint_->timestamp_tz("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` timestamp not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeStampTzWithDefault) {
  blueprint_->set_table("users");
  auto &column = blueprint_->timestamp_tz("foo");
  column["default"] = "2018-5-19 15:12:07";

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` timestamp not null default \
'2018-5-19 15:12:07'", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeStamps) {
  blueprint_->set_table("users");
  blueprint_->timestamps();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `created_at` timestamp null, \
add `updated_at` timestamp null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingTimeStampsTz) {
  blueprint_->set_table("users");
  blueprint_->timestamps_tz();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `created_at` timestamp null, \
add `updated_at` timestamp null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingRememberToken) {
  blueprint_->set_table("users");
  blueprint_->remember_token();

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `remember_token` varchar(100) null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingBinary) {
  blueprint_->set_table("users");
  blueprint_->binary("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` blob not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingUuid) {
  blueprint_->set_table("users");
  blueprint_->uuid("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` char(36) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingIpAddress) {
  blueprint_->set_table("users");
  blueprint_->ip_address("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(45) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingMacAddress) {
  blueprint_->set_table("users");
  blueprint_->mac_address("foo");

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(17) not null", 
               statements[0].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testAddingComment) {
  blueprint_->set_table("users");
  auto &column = blueprint_->string("foo");
  column["comment"] = "Escape ' when using words like it's";

  auto statements = blueprint_->to_sql(connection_.get(), mysql_grammar_.get());

  ASSERT_TRUE(1 == statements.size());

  ASSERT_STREQ("alter table `users` add `foo` varchar(255) not null comment \
'Escape \\' when using words like it\\'s'", 
               statements[0].c_str());
}
