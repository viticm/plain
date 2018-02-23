#include "gtest/gtest.h"
#include "pf/engine/kernel.h"
#include "pf/db/query/grammars/grammar.h"
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

     auto connection = new pf_db::Connection(engine_.get_db());
     unique_move(pf_db::Connection, connection, connection_);
     auto builder = new Builder(connection_.get(), nullptr);
     unique_move(Builder, builder, builder_);

   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:
   virtual void SetUp() {
     builder_->clear();
   }
   virtual void TearDown() {
   }

 protected:
   static pf_engine::Kernel engine_;
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<Builder> builder_;

};

pf_engine::Kernel DBQueryBuilder::engine_;
std::unique_ptr<pf_db::Connection> DBQueryBuilder::connection_{nullptr};
std::unique_ptr<Builder> DBQueryBuilder::builder_{nullptr};

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

}
