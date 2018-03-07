#include "gtest/gtest.h"
#include "pf/engine/kernel.h"
#include "pf/db/query/grammars/grammar.h"
#include "pf/db/query/grammars/mysql_grammar.h"
#include "pf/db/query/grammars/postgres_grammar.h"
#include "pf/db/query/grammars/sqlserver_grammar.h"
#include "pf/db/query/grammars/sqlite_grammar.h"
#include "pf/db/connection.h"
#include "pf/db/query/builder.h"
#include "pf/support/helpers.h"

enum {
  kDBTypeODBC = 1,
};

using namespace pf_db::query;
using namespace pf_basic::type;

class DBQueryBuilder : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     GLOBALS["log.print"] = false; //First forbid the log print.

     GLOBALS["default.db.open"] = true;
     GLOBALS["default.db.type"] = kDBTypeODBC;
     GLOBALS["default.db.name"] = "pf_test";
     GLOBALS["default.db.user"] = "root";
     GLOBALS["default.db.password"] = "mysql";

     engine_.add_libraryload("pf_plugin_odbc", {kDBTypeODBC});

     engine_.init();

     //Normal.
     auto connection = new pf_db::Connection(engine_.get_db());
     unique_move(pf_db::Connection, connection, connection_);
     auto builder = new Builder(connection_.get(), nullptr);
     unique_move(Builder, builder, builder_);
     
     //Mysql.
     auto mysql_grammar = new grammars::MysqlGrammar();
     unique_move(grammars::Grammar, mysql_grammar, mysql_grammar_);
     auto mysql_builder = new Builder(connection_.get(), mysql_grammar_.get());
     unique_move(Builder, mysql_builder, mysql_builder_);

     //Postgres.
     auto postgres_grammar = new grammars::PostgresGrammar();
     unique_move(grammars::Grammar, postgres_grammar, postgres_grammar_);
     auto postgres_builder = 
       new Builder(connection_.get(), postgres_grammar_.get());
     unique_move(Builder, postgres_builder, postgres_builder_);

     //Sqlserver.
     auto sqlserver_grammar = new grammars::SqlserverGrammar();
     unique_move(grammars::Grammar, sqlserver_grammar, sqlserver_grammar_);
     auto sqlserver_builder = 
       new Builder(connection_.get(), sqlserver_grammar_.get());
     unique_move(Builder, sqlserver_builder, sqlserver_builder_);

     //Sqlite.
     auto sqlite_grammar = new grammars::SqliteGrammar();
     unique_move(grammars::Grammar, sqlite_grammar, sqlite_grammar_);
     auto sqlite_builder = 
       new Builder(connection_.get(), sqlite_grammar_.get());
     unique_move(Builder, sqlite_builder, sqlite_builder_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:
   virtual void SetUp() {
     builder_->clear();
     mysql_builder_->clear();
     postgres_builder_->clear();
     sqlite_builder_->clear();
     sqlserver_builder_->clear();
   }
   virtual void TearDown() {
   }

 protected:
   static pf_engine::Kernel engine_;
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> mysql_grammar_;
   static std::unique_ptr<grammars::Grammar> postgres_grammar_;
   static std::unique_ptr<grammars::Grammar> sqlserver_grammar_;
   static std::unique_ptr<grammars::Grammar> sqlite_grammar_;
   static std::unique_ptr<Builder> builder_;
   static std::unique_ptr<Builder> mysql_builder_;
   static std::unique_ptr<Builder> postgres_builder_;
   static std::unique_ptr<Builder> sqlserver_builder_;
   static std::unique_ptr<Builder> sqlite_builder_;

};

pf_engine::Kernel DBQueryBuilder::engine_;
std::unique_ptr<pf_db::Connection> DBQueryBuilder::connection_{nullptr};
std::unique_ptr<Builder> DBQueryBuilder::builder_{nullptr};
std::unique_ptr<Builder> DBQueryBuilder::mysql_builder_{nullptr};
std::unique_ptr<Builder> DBQueryBuilder::postgres_builder_{nullptr};
std::unique_ptr<Builder> DBQueryBuilder::sqlserver_builder_{nullptr};
std::unique_ptr<Builder> DBQueryBuilder::sqlite_builder_{nullptr};
std::unique_ptr<grammars::Grammar> DBQueryBuilder::mysql_grammar_{nullptr};
std::unique_ptr<grammars::Grammar> DBQueryBuilder::postgres_grammar_{nullptr};
std::unique_ptr<grammars::Grammar> DBQueryBuilder::sqlserver_grammar_{nullptr};
std::unique_ptr<grammars::Grammar> DBQueryBuilder::sqlite_grammar_{nullptr};

TEST_F(DBQueryBuilder, construct) {
  Builder object(nullptr, nullptr);
  pf_db::Connection connection(engine_.get_db());
  Builder builder_test1(&connection, nullptr);
  grammars::Grammar grammar;
  Builder builder_test2(&connection, &grammar);
}

TEST_F(DBQueryBuilder, testBasicSelect) {
  builder_->select({"*"}).from("users");
  ASSERT_STREQ("select * from \"users\"", builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicSelectWithGetColumns) {
  builder_->from("users").get();
  ASSERT_TRUE(builder_->columns_.empty());
  
  ASSERT_STREQ("select * from \"users\"", builder_->to_sql().c_str());
  ASSERT_TRUE(builder_->columns_.empty());
}

TEST_F(DBQueryBuilder, testBasicSelectUseWritePdo) {

}

TEST_F(DBQueryBuilder, testBasicTableWrappingProtectsQuotationMarks) {
  builder_->select({"*"}).from("some\"table");
  ASSERT_STREQ("select * from \"some\"\"table\"", builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testAliasWrappingAsWholeConstant) {
  builder_->select({"x.y as foo.bar"}).from("baz");
  ASSERT_STREQ("select \"x\".\"y\" as \"foo.bar\" from \"baz\"", 
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testAliasWrappingWithSpacesInDatabaseName) {
  builder_->select({"w x.y.z as foo.bar"}).from("baz");
  ASSERT_STREQ("select \"w x\".\"y\".\"z\" as \"foo.bar\" from \"baz\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testAddingSelects) {
  builder_->select({"foo"}).
            add_select({"bar"}).add_select({"baz", "boom"}).from("users");
  ASSERT_STREQ("select \"foo\", \"bar\", \"baz\", \"boom\" from \"users\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicSelectWithPrefix) {
  builder_->get_grammar()->set_table_prefix("prefix_");
  builder_->select({"*"}).from("users");
  ASSERT_STREQ("select * from \"prefix_users\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicSelectDistinct) {
  builder_->distinct().select({"foo", "bar"}).from("users");
  ASSERT_STREQ("select distinct \"foo\", \"bar\" from \"users\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicAlias) {
  builder_->select({"foo as bar"}).from("users");
  ASSERT_STREQ("select \"foo\" as \"bar\" from \"users\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testAliasWithPrefix) {
  builder_->get_grammar()->set_table_prefix("prefix_");
  builder_->select({"*"}).from("users as people");
  ASSERT_STREQ("select * from \"prefix_users\" as \"prefix_people\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testJoinAliasesWithPrefix) {
  builder_->get_grammar()->set_table_prefix("prefix_");
  builder_->select({"*"}).from("services").join(
      "translations AS t", "t.item_id", "=", "services.id");
   ASSERT_STREQ(
       "select * from \"prefix_services\" inner join \"prefix_translations\" \
as \"prefix_t\" on \"prefix_t\".\"item_id\" = \"prefix_services\".\"id\"",
       builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicTableWrapping) {
  builder_->select({"*"}).from("public.users");
  ASSERT_STREQ("select * from \"public\".\"users\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testWhenCallback) {
  auto callback = [](Builder *query, const variable_t &condition) {
    ASSERT_TRUE(condition.get<bool>());
    query->where("id", "=", 1);
  };
  builder_->select({"*"}).from("users").when(true, callback).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").when(false, callback).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"email\" = ?",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testWhenCallbackWithReturn) {

}

void assertEquals(
    const variable_array_t &a, const variable_array_t &b, int32_t line = -1) {
  if (line != -1)
    std::cout << "assertEquals: " << line << std::endl;
  ASSERT_TRUE(a.size() == b.size());
  for (size_t i = 0; i < a.size(); ++i)
    ASSERT_STREQ(a[i].data.c_str(), b[i].data.c_str());
}

TEST_F(DBQueryBuilder, testWhenCallbackWithDefault) {
  auto callback = [](Builder *query, const variable_t &condition) {
    ASSERT_STREQ(condition.c_str(), "truthy");
    query->where("id", "=", 1);
  };
  auto def = [](Builder *query, const variable_t &condition) {
    ASSERT_TRUE(condition == 0);
    query->where("id", "=", 2);
  };

  builder_->select({"*"}).
            from("users").when("truthy", callback, def).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());

  assertEquals({1, "foo"}, builder_->get_bindings(), __LINE__);

  builder_->clear();

  builder_->select({"*"}).
            from("users").when(0, callback, def).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());

  assertEquals({2, "foo"}, builder_->get_bindings(), __LINE__);
}

TEST_F(DBQueryBuilder, testUnlessCallback) {
  auto callback = [](Builder *query, const variable_t &condition) {
    ASSERT_FALSE(condition.get<bool>());
    query->where("id", "=", 1);
  };

  builder_->select({"*"}).
            from("users").unless(false, callback).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());


  builder_->clear();
  builder_->select({"*"}).
            from("users").unless(true, callback).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"email\" = ?",
               builder_->to_sql().c_str());

}

TEST_F(DBQueryBuilder, testUnlessCallbackWithReturn) {

}

TEST_F(DBQueryBuilder, testUnlessCallbackWithDefault) {
  auto callback = [](Builder *query, const variable_t &condition) {
    ASSERT_TRUE(condition == 0);
    query->where("id", "=", 1);
  };
  auto def = [](Builder *query, const variable_t &condition) {
    ASSERT_STREQ(condition.c_str(), "truthy");
    query->where("id", "=", 2);
  };

  builder_->select({"*"}).
            from("users").unless(0, callback, def).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());

  assertEquals({1, "foo"}, builder_->get_bindings(), __LINE__);

  builder_->clear();

  builder_->select({"*"}).
            from("users").unless("truthy", callback, def).where("email", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());

  assertEquals({2, "foo"}, builder_->get_bindings(), __LINE__);
}

TEST_F(DBQueryBuilder, testTapCallback) {
  auto callback = [](Builder *query) {
    query->where("id", "=", 1);
  };

  builder_->select({"*"}).from("users").tap(callback).where("email", "foo"); 
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? and \"email\" = ?",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicWheres) {
  builder_->select({"*"}).from("users").where("id", "=", 1);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testMySqlWrappingProtectsQuotationMarks) {
/**
  builder_->select({"*"}).from("some`table");
  ASSERT_STREQ("select * from `some``table`",
               builder_->to_sql().c_str());
**/
}

TEST_F(DBQueryBuilder, testDateBasedWheresAcceptsTwoArguments) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where_date("created_at", "1");
  ASSERT_STREQ("select * from `users` where date(`created_at`) = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").where_day("created_at", "1");
  ASSERT_STREQ("select * from `users` where day(`created_at`) = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").where_month("created_at", "1");
  ASSERT_STREQ("select * from `users` where month(`created_at`) = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").where_year("created_at", "1");
  ASSERT_STREQ("select * from `users` where year(`created_at`) = ?",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testWhereDayMySql) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where_day("created_at", "=", 1);
  ASSERT_STREQ("select * from `users` where day(`created_at`) = ?",
               builder->to_sql().c_str());
  assertEquals({1}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereMonthMySql) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where_month("created_at", "=", 5);
  ASSERT_STREQ("select * from `users` where month(`created_at`) = ?",
               builder->to_sql().c_str());
  assertEquals({5}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereYearMySql) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where_year("created_at", "=", 2018);
  ASSERT_STREQ("select * from `users` where year(`created_at`) = ?",
               builder->to_sql().c_str());
  assertEquals({2018}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereTimeMySql) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where_time("created_at", "=", "22:00");
  ASSERT_STREQ("select * from `users` where time(`created_at`) = ?",
               builder->to_sql().c_str());
  assertEquals({"22:00"}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereDatePostgres) {
  auto builder = postgres_builder_.get();
  builder->select({"*"}).from("users").where_date("created_at", "=", "2018-02-28");
  ASSERT_STREQ("select * from \"users\" where \"created_at\"::date = ?",
               builder->to_sql().c_str());
  assertEquals({"2018-02-28"}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereDayPostgres) {
  auto builder = postgres_builder_.get();
  builder->select({"*"}).from("users").where_day("created_at", "=", 1);
  ASSERT_STREQ("select * from \"users\" where extract(day from \"created_at\") = ?",
               builder->to_sql().c_str());
  assertEquals({1}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereMonthPostgres) {
  auto builder = postgres_builder_.get();
  builder->select({"*"}).from("users").where_month("created_at", "=", 5);
  ASSERT_STREQ("select * from \"users\" where extract(month from \"created_at\") = ?",
               builder->to_sql().c_str());
  assertEquals({5}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereYearPostgres) {
  auto builder = postgres_builder_.get();
  builder->select({"*"}).from("users").where_year("created_at", "=", 2018);
  ASSERT_STREQ("select * from \"users\" where extract(year from \"created_at\") = ?",
               builder->to_sql().c_str());
  assertEquals({2018}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereDaySqlite) {
  auto builder = sqlite_builder_.get();
  builder->select({"*"}).from("users").where_day("created_at", "=", 1);
  ASSERT_STREQ("select * from \"users\" where strftime('%d', \"created_at\") = ?",
               builder->to_sql().c_str());
  assertEquals({1}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereMonthSqlite) {
  auto builder = sqlite_builder_.get();
  builder->select({"*"}).from("users").where_month("created_at", "=", 5);
  ASSERT_STREQ("select * from \"users\" where strftime('%m', \"created_at\") = ?",
               builder->to_sql().c_str());
  assertEquals({5}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereYearSqlite) {
  auto builder = sqlite_builder_.get();
  builder->select({"*"}).from("users").where_year("created_at", "=", 2018);
  ASSERT_STREQ("select * from \"users\" where strftime('%Y', \"created_at\") = ?",
               builder->to_sql().c_str());
  assertEquals({2018}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereDaySqlServer) {
  auto builder = sqlserver_builder_.get();
  builder->select({"*"}).from("users").where_day("created_at", "=", 1);
  ASSERT_STREQ("select * from [users] where day([created_at]) = ?",
               builder->to_sql().c_str());
  assertEquals({1}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereMonthSqlServer) {
  auto builder = sqlserver_builder_.get();
  builder->select({"*"}).from("users").where_month("created_at", "=", 5);
  ASSERT_STREQ("select * from [users] where month([created_at]) = ?",
               builder->to_sql().c_str());
  assertEquals({5}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereYearSqlServer) {
  auto builder = sqlserver_builder_.get();
  builder->select({"*"}).from("users").where_year("created_at", "=", 2018);
  ASSERT_STREQ("select * from [users] where year([created_at]) = ?",
               builder->to_sql().c_str());
  assertEquals({2018}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereBetweens) {
  builder_->select({"*"}).from("users").where_between("id", {1, 2});
  ASSERT_STREQ("select * from \"users\" where \"id\" between ? and ?",
               builder_->to_sql().c_str());
  assertEquals({1, 2}, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users").where_notbetween("id", {1, 2});
  ASSERT_STREQ("select * from \"users\" where \"id\" not between ? and ?",
               builder_->to_sql().c_str());
  assertEquals({1, 2}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testBasicOrWheres) {
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where("email", "=", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"email\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1, "foo"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testRawWheres) {
  builder_->select({"*"}).
            from("users").where_raw("id = ? or email = ?", {1, "foo"});
  ASSERT_STREQ("select * from \"users\" where id = ? or email = ?",
               builder_->to_sql().c_str());
  assertEquals({1, "foo"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testRawOrWheres) {
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where_raw("email = ?", {"foo"});
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or email = ?",
               builder_->to_sql().c_str());
  assertEquals({1, "foo"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testBasicWhereIns) {
  builder_->select({"*"}).from("users").where_in("id", {1, 2, 3});
  ASSERT_STREQ("select * from \"users\" where \"id\" in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({1, 2, 3}, builder_->get_bindings(), __LINE__);

  builder_->clear();
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where_in("id", {1, 2, 3});
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"id\" in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({1, 1, 2, 3}, builder_->get_bindings(), __LINE__);
}

TEST_F(DBQueryBuilder, testBasicWhereNotIns) {
  builder_->select({"*"}).from("users").where_notin("id", {1, 2, 3});
  ASSERT_STREQ("select * from \"users\" where \"id\" not in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({1, 2, 3}, builder_->get_bindings(), __LINE__);

  builder_->clear();
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where_notin("id", {1, 2, 3});
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"id\" not in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({1, 1, 2, 3}, builder_->get_bindings(), __LINE__);
}


TEST_F(DBQueryBuilder, testRawWhereIns) {
  using namespace pf_db;
  variable_array_t a{raw(1)};
  builder_->select({"*"}).from("users").where_in("id", a);
  ASSERT_STREQ("select * from \"users\" where \"id\" in (1)",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where_in("id", a);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"id\" in (1)",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testEmptyWhereIns) {
  using namespace pf_db;
  variable_array_t a;
  builder_->select({"*"}).from("users").where_in("id", a);
  ASSERT_STREQ("select * from \"users\" where 0 = 1",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where_in("id", a);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or 0 = 1",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testEmptyWhereNotIns) {
  using namespace pf_db;
  variable_array_t a;
  builder_->select({"*"}).from("users").where_notin("id", a);
  ASSERT_STREQ("select * from \"users\" where 1 = 1",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).
            from("users").where("id", "=", 1).or_where_notin("id", a);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or 1 = 1",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicWhereColumn) {
  builder_->select({"*"}).
            from("users").where_column("first_name", "last_name").
            or_where_column("first_name", "middle_name");
  ASSERT_STREQ("select * from \"users\" where \"first_name\" = \"last_name\" \
or \"first_name\" = \"middle_name\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).
            from("users").where_column("updated_at", ">", "created_at");
  ASSERT_STREQ("select * from \"users\" where \"updated_at\" > \"created_at\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testArrayWhereColumn) {
  std::vector<variable_array_t> conditions = {
    {"first_name", "last_name"},
    {"updated_at", ">", "created_at"},
  };
  builder_->select({"*"}).from("users").where_column(conditions);
  ASSERT_STREQ("select * from \"users\" where (\"first_name\" = \"last_name\" \
and \"updated_at\" > \"created_at\")",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testUnions) {

  auto builder = mysql_builder_.get();

  builder_->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("users").where("id", "=", 2);
  builder_->_union(union_builder);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? union select * from \
\"users\" where \"id\" = ?",
               builder_->to_sql().c_str());

  builder->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder1(
      new Builder(connection_.get(), mysql_grammar_.get()));
  union_builder1->select({"*"}).from("users").where("id", "=", 2);
  builder->_union(union_builder1);
  ASSERT_STREQ("(select * from `users` where `id` = ?) union (select * from \
`users` where `id` = ?)",
               builder->to_sql().c_str());
 
  builder->clear();
  std::string expected_sql{"(select `a` from `t1` where `a` = ? and `b` = ?) "
  "union (select `a` from `t2` where `a` = ? and `b` = ?) order by `a` "
  "asc limit 10"};
  std::unique_ptr<Builder> union_builder2(
      new Builder(connection_.get(), mysql_grammar_.get()));
  union_builder2->select({"a"}).from("t2").where("a", 11).where("b", 2);
  builder->select({"a"}).
           from("t1").where("a", 10).where("b", 1).
           _union(union_builder2).order_by("a").limit(10);
  ASSERT_STREQ(expected_sql.c_str(),
               builder->to_sql().c_str());
  assertEquals({10, 1, 11, 2}, builder->get_bindings());
  
  //SQLite...
  expected_sql = "select * from (select \"name\" from \"users\" where \"id\" \
= ?) union select * from (select \"name\" from \"users\" where \"id\" = ?)";
  sqlite_builder_->select({"name"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder3(
      new Builder(connection_.get(), sqlite_grammar_.get()));
  union_builder3->select({"name"}).from("users").where("id", "=", 2);
  sqlite_builder_->_union(union_builder3);
  ASSERT_STREQ(expected_sql.c_str(),
               sqlite_builder_->to_sql().c_str());
  assertEquals({1, 2}, sqlite_builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testUnionAlls) {
  builder_->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("users").where("id", "=", 2);
  builder_->union_all(union_builder);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? union all select * from \
\"users\" where \"id\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1, 2}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testMultipleUnions) {
  builder_->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("users").where("id", "=", 2);
  std::unique_ptr<Builder> union_builder1(new Builder(connection_.get(), nullptr));
  union_builder1->select({"*"}).from("users").where("id", "=", 3);
  builder_->_union(union_builder);
  builder_->_union(union_builder1);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? union select * from \
\"users\" where \"id\" = ? union select * from \"users\" where \"id\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1, 2, 3}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testMultipleUnionAlls) {
  builder_->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("users").where("id", "=", 2);
  std::unique_ptr<Builder> union_builder1(new Builder(connection_.get(), nullptr));
  union_builder1->select({"*"}).from("users").where("id", "=", 3);
  builder_->union_all(union_builder);
  builder_->union_all(union_builder1);
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? union all select * from \
\"users\" where \"id\" = ? union all select * from \"users\" where \"id\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1, 2, 3}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testUnionOrderBys) {
  builder_->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("users").where("id", "=", 2);
  builder_->_union(union_builder);
  builder_->order_by("id", "desc");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? union select * from \
\"users\" where \"id\" = ? order by \"id\" desc",
               builder_->to_sql().c_str());
  assertEquals({1, 2}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testUnionLimitsAndOffsets) {
  builder_->select({"*"}).from("users");
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("dogs");
  builder_->_union(union_builder);
  builder_->skip(5).take(10);
  ASSERT_STREQ("select * from \"users\" union select * from \"dogs\" limit 10 offset 5",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testUnionWithJoin) {
  builder_->select({"*"}).from("users");
  std::unique_ptr<Builder> union_builder(new Builder(connection_.get(), nullptr));
  union_builder->select({"*"}).from("dogs");
  union_builder->join("breeds", [](Builder *join){
    join->on("dogs.breed_id", "=", "breeds.id").where("breeds.is_native", "=", 1);
  });
  builder_->_union(union_builder);
  ASSERT_STREQ("select * from \"users\" union select * from \"dogs\" inner join \
\"breeds\" on \"dogs\".\"breed_id\" = \"breeds\".\"id\" and \"breeds\".\"is_native\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testMySqlUnionOrderBys) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where("id", "=", 1);
  std::unique_ptr<Builder> union_builder(
      new Builder(connection_.get(), mysql_grammar_.get()));
  union_builder->select({"*"}).from("users").where("id", "=", 2);
  builder->_union(union_builder);
  builder->order_by("id", "desc");
  ASSERT_STREQ("(select * from `users` where `id` = ?) union (select * from \
`users` where `id` = ?) order by `id` desc",
               builder->to_sql().c_str());
  assertEquals({1, 2}, builder->get_bindings());
}

TEST_F(DBQueryBuilder, testMySqlUnionLimitsAndOffsets) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users");
  std::unique_ptr<Builder> union_builder(
      new Builder(connection_.get(), mysql_grammar_.get()));
  union_builder->select({"*"}).from("dogs");
  builder->_union(union_builder);
  builder->skip(5).take(10);
  ASSERT_STREQ("(select * from `users`) union (select * from `dogs`) \
limit 10 offset 5",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testSubSelectWhereIns) {
  builder_->select({"*"}).from("users").where_in("id", [](Builder *query){
    query->select({"id"}).from("users").where("age", ">", 25).take(3);
  });
  ASSERT_STREQ("select * from \"users\" where \"id\" in (select \"id\" from \
\"users\" where \"age\" > ? limit 3)",
               builder_->to_sql().c_str());
  assertEquals({25}, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users").where_notin("id", [](Builder *query){
    query->select({"id"}).from("users").where("age", ">", 25).take(3);
  });
  ASSERT_STREQ("select * from \"users\" where \"id\" not in (select \"id\" from \
\"users\" where \"age\" > ? limit 3)",
               builder_->to_sql().c_str());
  assertEquals({25}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testBasicWhereNulls) {
  builder_->select({"*"}).from("users").where_null("id");
  ASSERT_STREQ("select * from \"users\" where \"id\" is null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").where("id", "=", 1).or_where_null("id");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"id\" is null",
               builder_->to_sql().c_str());
  assertEquals({1}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testBasicWhereNotNulls) {
  builder_->select({"*"}).from("users").where_notnull("id");
  ASSERT_STREQ("select * from \"users\" where \"id\" is not null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").where("id", "=", 1).or_where_notnull("id");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"id\" is not null",
               builder_->to_sql().c_str());
  assertEquals({1}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testGroupBys) {
  using namespace pf_db;
  builder_->select({"*"}).from("users").group_by({"email"});
  ASSERT_STREQ("select * from \"users\" group by \"email\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").group_by({"id", "email"});
  ASSERT_STREQ("select * from \"users\" group by \"id\", \"email\"",
               builder_->to_sql().c_str());

  builder_->clear();
  variable_array_t groups;
  groups.emplace_back(raw("DATE(created_at)"));
  builder_->select({"*"}).from("users").group_by(groups);
  ASSERT_STREQ("select * from \"users\" group by DATE(created_at)",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testOrderBys) {
  builder_->select({"*"}).from("users").order_by("email").order_by("age", "desc");
  ASSERT_STREQ("select * from \"users\" order by \"email\" asc, \"age\" desc",
               builder_->to_sql().c_str());

  builder_->orders_ = {};
  ASSERT_STREQ("select * from \"users\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").
            order_by("email").order_byraw("\"age\" ? desc", {"foo"});
  ASSERT_STREQ("select * from \"users\" order by \"email\" asc, \"age\" ? desc",
               builder_->to_sql().c_str());
  assertEquals({"foo"}, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users").order_bydesc("name");
  ASSERT_STREQ("select * from \"users\" order by \"name\" desc",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testHavings) {
  using namespace pf_db;
  builder_->select({"*"}).from("users").having("email", ">", 1);
  ASSERT_STREQ("select * from \"users\" having \"email\" > ?",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users")
                  .or_having("email", "=", "test@example.com")
                  .or_having("email", "=", "test2@example.com");
  ASSERT_STREQ("select * from \"users\" having \"email\" = ? or \"email\" = ?",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").group_by({"email"}).having("email", ">", 1);
  ASSERT_STREQ("select * from \"users\" group by \"email\" having \"email\" > ?",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"email as foo_email"})
            .from("users").having("foo_email", ">", 1);
  ASSERT_STREQ("select \"email\" as \"foo_email\" from \"users\" having \
\"foo_email\" > ?",
               builder_->to_sql().c_str());

  builder_->clear();
  variable_array_t columns;
  columns.emplace_back("category");
  columns.emplace_back(raw("count(*) as \"total\""));
  auto value1 = raw(3);
  builder_->select(columns).from("item")
            .where("department", "=", "popular").group_by({"category"})
            .having("total", ">", value1);
  ASSERT_STREQ("select \"category\", count(*) as \"total\" from \"item\" \
where \"department\" = ? group by \"category\" having \"total\" > 3",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select(columns).from("item")
            .where("department", "=", "popular").group_by({"category"})
            .having("total", ">", 3);
  ASSERT_STREQ("select \"category\", count(*) as \"total\" from \"item\" \
where \"department\" = ? group by \"category\" having \"total\" > ?",
               builder_->to_sql().c_str());
  assertEquals({"popular", 3}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testHavingShortcut) {
  builder_->select({"*"}).from("users").having("email", 1).or_having("email", 2);
  ASSERT_STREQ("select * from \"users\" having \"email\" = ? or \"email\" = ?",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testHavingFollowedBySelectGet) {

}

TEST_F(DBQueryBuilder, testRawHavings) {
  builder_->select({"*"}).from("users").having_raw("user_foo < user_bar");
  ASSERT_STREQ("select * from \"users\" having user_foo < user_bar",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users")
            .having("baz", "=", 1).or_having_raw("user_foo < user_bar");
  ASSERT_STREQ("select * from \"users\" having \"baz\" = ? or user_foo \
< user_bar",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testLimitsAndOffsets) {
  builder_->select({"*"}).from("users").offset(5).limit(10);
  ASSERT_STREQ("select * from \"users\" limit 10 offset 5",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").skip(5).take(10);
  ASSERT_STREQ("select * from \"users\" limit 10 offset 5",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").skip(0).take(0);
  ASSERT_STREQ("select * from \"users\" limit 0 offset 0",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").skip(-5).take(-10);
  ASSERT_STREQ("select * from \"users\" offset 0",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testForPage) {
  builder_->select({"*"}).from("users").for_page(2, 15);
  ASSERT_STREQ("select * from \"users\" limit 15 offset 15",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").for_page(0, 15);
  ASSERT_STREQ("select * from \"users\" limit 15 offset 0",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").for_page(-2, 15);
  ASSERT_STREQ("select * from \"users\" limit 15 offset 0",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").for_page(2, 0);
  ASSERT_STREQ("select * from \"users\" limit 0 offset 0",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").for_page(0, 0);
  ASSERT_STREQ("select * from \"users\" limit 0 offset 0",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").for_page(-2, 0);
  ASSERT_STREQ("select * from \"users\" limit 0 offset 0",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testGetCountForPaginationWithBindings) {

}

TEST_F(DBQueryBuilder, testGetCountForPaginationWithColumnAliases) {

}

TEST_F(DBQueryBuilder, testWhereShortcut) {
  builder_->select({"*"}).from("users").where("id", 1).or_where("name", "foo");
  ASSERT_STREQ("select * from \"users\" where \"id\" = ? or \"name\" = ?",
               builder_->to_sql().c_str());
  assertEquals({1, "foo"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereWithArrayConditions) {
  builder_->select({"*"}).from("users").where({{"foo", 1}, {"bar", 2}});
  ASSERT_STREQ("select * from \"users\" where (\"foo\" = ? and \"bar\" = ?)",
               builder_->to_sql().c_str());
  assertEquals({1, 2}, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users").where({{"foo", 1}, {"bar", ">", 2}});
  ASSERT_STREQ("select * from \"users\" where (\"foo\" = ? and \"bar\" > ?)",
               builder_->to_sql().c_str());
  assertEquals({1, 2}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testNestedWheres) {
  builder_->select({"*"}).from("users").where("email", "=", "foo")
            .or_where([](Builder *query){
    query->where("name", "=", "bar").where("age", "=", 25); 
  });
  ASSERT_STREQ("select * from \"users\" where \"email\" = ? or (\"name\" = ? \
and \"age\" = ?)",
               builder_->to_sql().c_str());
  assertEquals({"foo", "bar", 25}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testFullSubSelects) {
  using namespace pf_db;
  builder_->select({"*"}).from("users").where("email", "=", "foo")
            .or_wheref("id", "=", [](Builder *query){
    auto column = raw("max(id)");
    variable_array_t columns{column,};
    query->select(columns).from("users").where("email", "=", "bar");
  });
  ASSERT_STREQ("select * from \"users\" where \"email\" = ? or \"id\" = \
(select max(id) from \"users\" where \"email\" = ?)",
               builder_->to_sql().c_str());
  assertEquals({"foo", "bar"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testWhereExists) {
  using namespace pf_db;
  builder_->select({"*"}).from("orders").where_exists([](Builder *query){
    query->select({"*"}).from("products")
           .where("products.id", "=", raw("\"orders\".\"id\""));
  });
  ASSERT_STREQ("select * from \"orders\" where exists (select * from \
\"products\" where \"products\".\"id\" = \"orders\".\"id\")",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("orders").where_not_exists([](Builder *query){
    query->select({"*"}).from("products")
           .where("products.id", "=", raw("\"orders\".\"id\""));
  });
  ASSERT_STREQ("select * from \"orders\" where not exists (select * from \
\"products\" where \"products\".\"id\" = \"orders\".\"id\")",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("orders").where("id", "=", 1)
            .or_where_exists([](Builder *query){
    query->select({"*"}).from("products")
           .where("products.id", "=", raw("\"orders\".\"id\""));
  });
  ASSERT_STREQ("select * from \"orders\" where \"id\" = ? or exists (select * from \
\"products\" where \"products\".\"id\" = \"orders\".\"id\")",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("orders").where("id", "=", 1)
            .or_where_not_exists([](Builder *query){
    query->select({"*"}).from("products")
           .where("products.id", "=", raw("\"orders\".\"id\""));
  });
  ASSERT_STREQ("select * from \"orders\" where \"id\" = ? or not exists (select * from \
\"products\" where \"products\".\"id\" = \"orders\".\"id\")",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testBasicJoins) {
  builder_->select({"*"}).from("users")
            .join("contacts", "users.id", "contacts.id");
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \
\"users\".\"id\" = \"contacts\".\"id\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users")
            .join("contacts", "users.id", "=", "contacts.id")
            .left_join("photos", "users.id", "=", "photos.id");
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \
\"users\".\"id\" = \"contacts\".\"id\" left join \"photos\" on \
\"users\".\"id\" = \"photos\".\"id\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users")
            .left_join_where("photos", "users.id", "=", "bar")
            .join_where("photos", "users.id", "=", "foo");
  ASSERT_STREQ("select * from \"users\" left join \"photos\" on \
\"users\".\"id\" = ? inner join \"photos\" on \"users\".\"id\" = ?",
               builder_->to_sql().c_str());
  assertEquals({"bar", "foo"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testCrossJoins) {
  builder_->select({"*"}).from("sizes").cross_join("colors");
  ASSERT_STREQ("select * from \"sizes\" cross join \"colors\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("tableB")
            .join("tableA", "tableA.column1", "=", "tableB.column2", "cross");
  ASSERT_STREQ("select * from \"tableB\" cross join \"tableA\" on \
\"tableA\".\"column1\" = \"tableB\".\"column2\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("tableB")
            .cross_join("tableA", "tableA.column1", "=", "tableB.column2");
  ASSERT_STREQ("select * from \"tableB\" cross join \"tableA\" on \
\"tableA\".\"column1\" = \"tableB\".\"column2\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testComplexJoin) {
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id")
          .or_on("users.name", "=", "contacts.name");
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" or \"users\".\"name\" = \"contacts\".\"name\"",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->where("users.id", "=", "foo")
          .or_where("users.name", "=", "bar");
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = ? or \"users\".\"name\" = ?",
               builder_->to_sql().c_str());
  assertEquals({"foo", "bar"}, builder_->get_bindings());

  // Run the assertions again
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = ? or \"users\".\"name\" = ?",
               builder_->to_sql().c_str());
  assertEquals({"foo", "bar"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testJoinWhereNull) {
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id").where_null("contacts.deleted_at");
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and \"contacts\".\"deleted_at\" is null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id").or_where_null("contacts.deleted_at");
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" or \"contacts\".\"deleted_at\" is null",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testJoinWhereNotNull) {
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id").where_notnull("contacts.deleted_at");
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and \"contacts\".\"deleted_at\" is not null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id").or_where_notnull("contacts.deleted_at");
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" or \"contacts\".\"deleted_at\" is not null",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testJoinWhereIn) {
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id")
          .where_in("contacts.name", {48, "baz", "null"});
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and \"contacts\".\"name\" in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({48, "baz", "null"}, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id")
          .or_where_in("contacts.name", {48, "baz", "null"});
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" or \"contacts\".\"name\" in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({48, "baz", "null"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testJoinWhereNotIn) {
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id")
          .where_notin("contacts.name", {48, "baz", "null"});
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and \"contacts\".\"name\" not in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({48, "baz", "null"}, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users").join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id")
          .or_where_notin("contacts.name", {48, "baz", "null"});
  });
  ASSERT_STREQ("select * from \"users\" inner join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" or \"contacts\".\"name\" not in (?, ?, ?)",
               builder_->to_sql().c_str());
  assertEquals({48, "baz", "null"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testJoinsWithNestedConditions) {
  builder_->select({"*"}).from("users").left_join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id").where([](Builder *_join){
      _join->where("contacts.country", "=", "US")
             .or_where("contacts.is_partner", "=", 1);
    });
  });
  ASSERT_STREQ("select * from \"users\" left join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and (\"contacts\".\"country\" = ? or \"contacts\".\
\"is_partner\" = ?)",
               builder_->to_sql().c_str());
  assertEquals({"US", 1}, builder_->get_bindings());
 
  builder_->clear();
  builder_->select({"*"}).from("users").left_join("contacts", [](Builder *join){
    join->on("users.id", "=", "contacts.id")
          .where("contacts.is_active", "=", 1).or_on([](Builder *_join){
      _join->or_where([](Builder *join1){
        join1->where("contacts.country", "=", "UK")
               .or_on("contacts.type", "=", "users.type");
      }).where([](Builder *join2){
        join2->where("contacts.country", "=", "US")
               .or_where_null("contacts.is_partner");
      });
    });
  });
  ASSERT_STREQ("select * from \"users\" left join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and \"contacts\".\"is_active\" = ? or \
((\"contacts\".\"country\" = ? or \"contacts\".\"type\" = \"users\".\"type\")\
 and (\"contacts\".\"country\" = ? or \"contacts\".\"is_partner\" is null))",
               builder_->to_sql().c_str());
  assertEquals({1, "UK", "US"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testJoinsWithAdvancedConditions) {
  builder_->select({"*"}).from("users").left_join("contacts", [](Builder *join){
    join->on("users.id", "contacts.id").where([](Builder *_join){
      _join->where("role", "admin").or_where_null("contacts.disabled")
             .or_where_raw("year(contacts.created_at) = 2018");
    });
  });
  ASSERT_STREQ("select * from \"users\" left join \"contacts\" on \"users\".\
\"id\" = \"contacts\".\"id\" and (\"role\" = ? or \"contacts\".\"disabled\" is \
null or year(contacts.created_at) = 2018)",
               builder_->to_sql().c_str());
  assertEquals({"admin"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testRawExpressionsInSelect) {
  using namespace pf_db;
  variable_array_t columns;
  columns.emplace_back(raw("substr(foo, 6)"));
  builder_->select(columns).from("users");
  ASSERT_STREQ("select substr(foo, 6) from \"users\"",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testFindReturnsFirstResultByID) {

}

TEST_F(DBQueryBuilder, testFirstMethodReturnsFirstResult) {

}

TEST_F(DBQueryBuilder, testListMethodsGetsArrayOfColumnValues) {

}

TEST_F(DBQueryBuilder, testImplode) {

}

TEST_F(DBQueryBuilder, testValueMethodReturnsSingleColumn) {

}

TEST_F(DBQueryBuilder, testAggregateFunctions) {

}

TEST_F(DBQueryBuilder, testSqlServerExists) {

}

TEST_F(DBQueryBuilder, testAggregateResetFollowedByGet) {

}

TEST_F(DBQueryBuilder, testAggregateResetFollowedBySelectGet) {

}

TEST_F(DBQueryBuilder, testAggregateResetFollowedByGetWithColumns) {

}

TEST_F(DBQueryBuilder, testAggregateWithSubSelect) {

}

TEST_F(DBQueryBuilder, testSubqueriesBindings) {
  using namespace pf_db;  
  std::unique_ptr<Builder> builder1(new Builder(connection_.get(), nullptr));
  std::unique_ptr<Builder> builder2(new Builder(connection_.get(), nullptr));
  builder1->select({"*"}).from("users").order_byraw("id = ?", {2});
  builder2->select({"*"}).from("users")
            .where("id", 3).group_by({"id"}).having("id", "!=", 4);
  builder_->group_by({"a"}).having("a", "=", 1)._union(builder1)._union(builder2);
  assertEquals({1, 2, 3, 4}, builder_->get_bindings(), __LINE__);
  
  builder_->clear();
  builder_->select({"*"}).from("users")
            .wheref("email", "=", [](Builder *query){
    variable_array_t columns;
    columns.emplace_back(raw("max(id)"));
    query->select(columns)
          .where("email", "=", "bar")
          .order_byraw("(email like ?", {"%.com"})
          .group_by({"id"}).having("id", "=", 4);
  }).or_where("id", "=", "foo").group_by({"id"}).having("id", "=", 5);

  assertEquals({"bar", 4, "%.com", "foo", 5}, builder_->get_bindings(), __LINE__);
}

TEST_F(DBQueryBuilder, testInsertMethod) {

}

TEST_F(DBQueryBuilder, testSQLiteMultipleInserts) {

}

TEST_F(DBQueryBuilder, testInsertGetIdMethod) {

}

TEST_F(DBQueryBuilder, testInsertGetIdMethodRemovesExpressions) {

}

TEST_F(DBQueryBuilder, testInsertMethodRespectsRawBindings) {

}

TEST_F(DBQueryBuilder, testMultipleInsertsWithExpressionValues) {

}

TEST_F(DBQueryBuilder, testUpdateMethod) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithJoins) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithJoinsOnSqlServer) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithJoinsOnMySql) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithJoinsOnSQLite) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithJoinsAndAliasesOnSqlServer) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithoutJoinsOnPostgres) {

}

TEST_F(DBQueryBuilder, testUpdateMethodWithJoinsOnPostgres) {

}

TEST_F(DBQueryBuilder, testUpdateMethodRespectsRaw) {

}

TEST_F(DBQueryBuilder, testUpdateOrInsertMethod) {

}

TEST_F(DBQueryBuilder, testDeleteMethod) {

}

TEST_F(DBQueryBuilder, testDeleteWithJoinMethod) {

}

TEST_F(DBQueryBuilder, testTruncateMethod) {

}

TEST_F(DBQueryBuilder, testPostgresInsertGetId) {

}

TEST_F(DBQueryBuilder, testMySqlWrapping) {
  mysql_builder_->select({"*"}).from("users");
  ASSERT_STREQ("select * from `users`",
               mysql_builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testMySqlUpdateWrappingJson) {

}

TEST_F(DBQueryBuilder, testMySqlUpdateWithJsonRemovesBindingsCorrectly) {

}

TEST_F(DBQueryBuilder, testMySqlWrappingJsonWithString) {
  mysql_builder_->select({"*"}).from("users").where("items->sku", "=", "foo-bar");
  ASSERT_STREQ("select * from `users` where `items`->'$.\"sku\"' = ?",
               mysql_builder_->to_sql().c_str());
  ASSERT_TRUE(1 == (*mysql_builder_->get_raw_bindings())["where"].size());
  ASSERT_STREQ("foo-bar", 
               (*mysql_builder_->get_raw_bindings())["where"][0].data.c_str());
}

TEST_F(DBQueryBuilder, testMySqlWrappingJsonWithInteger) {
  mysql_builder_->select({"*"}).from("users").where("items->price", "=", 1);
  ASSERT_STREQ("select * from `users` where `items`->'$.\"price\"' = ?",
               mysql_builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testMySqlWrappingJsonWithDouble) {
  mysql_builder_->select({"*"}).from("users").where("items->price", "=", 1.5);
  ASSERT_STREQ("select * from `users` where `items`->'$.\"price\"' = ?",
               mysql_builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testMySqlWrappingJsonWithBoolean) {
  mysql_builder_->select({"*"}).from("users").where("items->available", "=", true);
  ASSERT_STREQ("select * from `users` where `items`->'$.\"available\"' = true",
               mysql_builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testMySqlWrappingJsonWithBooleanAndIntegerThatLooksLikeOne) {
  mysql_builder_->select({"*"}).from("users")
                  .where("items->available", "=", true)
                  .where("items->active", "=", false)
                  .where("items->number_available", "=", 0);
  ASSERT_STREQ("select * from `users` where `items`->'$.\"available\"' = \
true and `items`->'$.\"active\"' = false and \
`items`->'$.\"number_available\"' = ?",
               mysql_builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testMySqlWrappingJson) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("users").where_raw("items->\"$.price\" = 1");
  ASSERT_STREQ("select * from `users` where items->\"$.price\" = 1",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"items->price"}).from("users")
           .where("items->price", "=", 1).order_by("items->price");
  ASSERT_STREQ("select `items`->'$.\"price\"' from `users` where \
`items`->'$.\"price\"' = ? order by `items`->'$.\"price\"' asc",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").where("items->price->in_usd", "=", 1);
  ASSERT_STREQ("select * from `users` where `items`->'$.\"price\".\"in_usd\"' = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users")
           .where("items->price->in_usd", "=", 1)
           .where("items->age", "=", 2);
  ASSERT_STREQ("select * from `users` where `items`->'$.\"price\".\"in_usd\"' \
= ? and `items`->'$.\"age\"' = ?",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testPostgresWrappingJson) {
  auto builder = postgres_builder_.get();
  builder->select({"items->price"}).from("users")
           .where("items->price", "=", 1).order_by("items->price");
  ASSERT_STREQ("select \"items\"->>'price' from \"users\" where \
\"items\"->>'price' = ? order by \"items\"->>'price' asc",
               builder->to_sql().c_str());
  
  builder->clear();
  builder->select({"*"}).from("users").where("items->price->in_usd", "=", 1);
  ASSERT_STREQ("select * from \"users\" where \"items\"->'price'->>'in_usd' = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users")
           .where("items->price->in_usd", "=", 1)
           .where("items->age", "=", 2);
  ASSERT_STREQ("select * from \"users\" where \"items\"->'price'->>'in_usd' \
= ? and \"items\"->>'age' = ?",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testSQLiteOrderBy) {
  auto builder = sqlite_builder_.get();
  builder->select({"*"}).from("users").order_by("email", "desc");
  ASSERT_STREQ("select * from \"users\" order by \"email\" desc",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testSqlServerLimitsAndOffsets) {
  auto builder = sqlserver_builder_.get();
  builder->select({"*"}).from("users").take(10);
  ASSERT_STREQ("select top 10 * from [users]",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").skip(10);
  ASSERT_STREQ("select * from (select *, row_number() over (order by (select 0))\
 as row_num from [users]) as temp_table where row_num >= 11",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").skip(10).take(10);
  ASSERT_STREQ("select * from (select *, row_number() over (order by (select 0))\
 as row_num from [users]) as temp_table where row_num between 11 and 20",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users").skip(10).take(10).order_by("email", "desc");
  ASSERT_STREQ("select * from (select *, row_number() over (order by [email] desc)\
 as row_num from [users]) as temp_table where row_num between 11 and 20",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testMergeWheresCanMergeWheresAndBindings) {

}

TEST_F(DBQueryBuilder, testProvidingNullWithOperatorsBuildsCorrectly) {
  using namespace pf_db;
  builder_->select({"*"}).from("users").where("foo", null());
  ASSERT_STREQ("select * from \"users\" where \"foo\" is null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").where("foo", "=", null());
  ASSERT_STREQ("select * from \"users\" where \"foo\" is null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").where("foo", "!=", null());
  ASSERT_STREQ("select * from \"users\" where \"foo\" is not null",
               builder_->to_sql().c_str());

  builder_->clear();
  builder_->select({"*"}).from("users").where("foo", "<>", null());
  ASSERT_STREQ("select * from \"users\" where \"foo\" is not null",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testDynamicWhere) {

}

TEST_F(DBQueryBuilder, testDynamicWhereIsNotGreedy) {

}

TEST_F(DBQueryBuilder, testCallTriggersDynamicWhere) {

}

TEST_F(DBQueryBuilder, testBuilderThrowsExpectedExceptionWithUndefinedMethod) {

}

TEST_F(DBQueryBuilder, testMySqlLock) {
  auto builder = mysql_builder_.get();
  builder->select({"*"}).from("foo").where("bar", "=", "baz").lock();
  ASSERT_STREQ("select * from `foo` where `bar` = ? for update",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("foo").where("bar", "=", "baz").lock(false);
  ASSERT_STREQ("select * from `foo` where `bar` = ? lock in share mode",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("foo")
           .where("bar", "=", "baz").lock("lock in share mode");
  ASSERT_STREQ("select * from `foo` where `bar` = ? lock in share mode",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testPostgresLock) {
  auto builder = postgres_builder_.get();
  builder->select({"*"}).from("foo").where("bar", "=", "baz").lock();
  ASSERT_STREQ("select * from \"foo\" where \"bar\" = ? for update",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("foo").where("bar", "=", "baz").lock(false);
  ASSERT_STREQ("select * from \"foo\" where \"bar\" = ? for share",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("foo")
           .where("bar", "=", "baz").lock("for key share");
  ASSERT_STREQ("select * from \"foo\" where \"bar\" = ? for key share",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testSqlServerLock) {
  auto builder = sqlserver_builder_.get();
  builder->select({"*"}).from("foo").where("bar", "=", "baz").lock();
  ASSERT_STREQ("select * from [foo] with(rowlock,updlock,holdlock) where [bar] = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("foo").where("bar", "=", "baz").lock(false);
  ASSERT_STREQ("select * from [foo] with(rowlock,holdlock) where [bar] = ?",
               builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("foo")
           .where("bar", "=", "baz").lock("with(holdlock)");
  ASSERT_STREQ("select * from [foo] with(holdlock) where [bar] = ?",
               builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testSelectWithLockUsesWritePdo) {

}

TEST_F(DBQueryBuilder, testBindingOrder) {
  std::string expected_sql{"select * from \"users\" inner join \"othertable\" \
on \"bar\" = ? where \"registered\" = ? group by \"city\" having \
\"population\" > ? order by match (\"foo\") against(?)"};
  variable_array_t expected_bindings{"foo", 1, 3, "bar"};

  builder_->select({"*"}).from("users").join("othertable", [](Builder *join){
    join->where("bar", "=", "foo");
  }).where("registered", 1).group_by({"city"}).having("population", ">", 3)
  .order_byraw("match (\"foo\") against(?)", {"bar"});
  ASSERT_STREQ(expected_sql.c_str(), builder_->to_sql().c_str());
  assertEquals(expected_bindings, builder_->get_bindings());

  builder_->clear();
  builder_->select({"*"}).from("users")
            .order_byraw("match (\"foo\") against(?)", {"bar"})
            .having("population", ">", 3).group_by({"city"})
            .where("registered", 1).join("othertable", [](Builder *join){
    join->where("bar", "=", "foo");
  });
  ASSERT_STREQ(expected_sql.c_str(), builder_->to_sql().c_str());
  assertEquals(expected_bindings, builder_->get_bindings(), __LINE__);
}

TEST_F(DBQueryBuilder, testAddBindingWithArrayMergesBindings) {
  builder_->add_bindings({"foo", "bar"});
  builder_->add_binding("baz");
  assertEquals({"foo", "bar", "baz"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testAddBindingWithArrayMergesBindingsInCorrectOrder) {
  builder_->add_bindings({"bar", "baz"}, "having");
  builder_->add_binding("foo", "where");
  assertEquals({"foo", "bar", "baz"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testMergeBuildersBindingOrder) {
  builder_->add_binding("foo", "where");
  builder_->add_binding("baz", "having");
  
  std::unique_ptr<Builder> other_builder(new Builder(connection_.get(), nullptr));
  other_builder->add_binding("bar", "where");

  builder_->merge_bindings(*other_builder.get());
  assertEquals({"foo", "bar", "baz"}, builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testSubSelect) {
  std::string expected_sql{"select \"foo\", \"bar\", (select \"baz\" from \
\"two\" where \"subkey\" = ?) as \"sub\" from \"one\" where \"key\" = ?"};
  variable_array_t expected_bindings{"subval", "val"};

  auto builder = postgres_builder_.get();
  builder->from("one").select({"foo", "bar"}).where("key", "=", "val");
  builder->select_sub([](Builder *query){
    query->from("two").select({"baz"}).where("subkey", "=", "subval");
  }, "sub");
  ASSERT_STREQ(expected_sql.c_str(), builder->to_sql().c_str());
  assertEquals(expected_bindings, builder->get_bindings(), __LINE__);

  builder->clear();
  std::unique_ptr<Builder> sub_builder(
      new Builder(connection_.get(), postgres_grammar_.get()));
  builder->from("one").select({"foo", "bar"}).where("key", "=", "val");
  sub_builder->from("two").select({"baz"}).where("subkey", "=", "subval");
  builder->select_sub(*sub_builder.get(), "sub");
  ASSERT_STREQ(expected_sql.c_str(), builder->to_sql().c_str());
  assertEquals(expected_bindings, builder->get_bindings(), __LINE__);
}

TEST_F(DBQueryBuilder, testSqlServerWhereDate) {
  sqlserver_builder_->select({"*"}).from("users")
                      .where_date("created_at", "=", "2018-03-07");
  ASSERT_STREQ("select * from [users] where cast([created_at] as date) = ?",
               sqlserver_builder_->to_sql().c_str());
  assertEquals({"2018-03-07"}, sqlserver_builder_->get_bindings());
}

TEST_F(DBQueryBuilder, testUppercaseLeadingBooleansAreRemoved) {
  builder_->select({"*"}).from("users").where("name", "=", "Taylor", "AND");
  ASSERT_STREQ("select * from \"users\" where \"name\" = ?",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testLowercaseLeadingBooleansAreRemoved) {
  builder_->select({"*"}).from("users").where("name", "=", "Taylor", "and");
  ASSERT_STREQ("select * from \"users\" where \"name\" = ?",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testCaseInsensitiveLeadingBooleansAreRemoved) {
  builder_->select({"*"}).from("users").where("name", "=", "Taylor", "And");
  ASSERT_STREQ("select * from \"users\" where \"name\" = ?",
               builder_->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testTableValuedFunctionAsTableInSqlServer) {
  auto builder = sqlserver_builder_.get();
  builder->select({"*"}).from("users()");
  ASSERT_STREQ("select * from [users]()", builder->to_sql().c_str());

  builder->clear();
  builder->select({"*"}).from("users(1,2)");
  ASSERT_STREQ("select * from [users](1,2)", builder->to_sql().c_str());
}

TEST_F(DBQueryBuilder, testChunkWithLastChunkComplete) {

}

TEST_F(DBQueryBuilder, testChunkWithLastChunkPartial) {

}

TEST_F(DBQueryBuilder, testChunkCanBeStoppedByReturningFalse) {

}
