#include "gtest/gtest.h"
#include "pf/engine/kernel.h"
#include "pf/db/query/grammars/grammar.h"
#include "pf/db/connection.h"
#include "pf/db/query/builder.h"


enum {
  kDBTypeODBC = 1,
};

using namespace pf_db::query;

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

/**
 public:
   virtual void SetUp() {
     std::cout << "my SetUp" << std::endl;
   }
   virtual void TearDown() {
     std::cout << "my TearDown" << std::endl;
   }
**/

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
