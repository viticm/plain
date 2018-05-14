#include "gtest/gtest.h"
#include "pf/engine/kernel.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/mysql_grammar.h"
#include "pf/db/schema/grammars/postgres_grammar.h"
#include "pf/db/schema/grammars/sqlite_grammar.h"
#include "pf/db/schema/grammars/sqlserver_grammar.h"
#include "pf/db/schema/blueprint.h"

enum {
  kDBTypeODBC = 1,
};

using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaBlueprint : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     GLOBALS["log.print"] = false; //First forbid the log print.

     GLOBALS["default.db.open"] = true;
     GLOBALS["default.db.type"] = kDBTypeODBC;
     GLOBALS["default.db.name"] = "pf_test";
     GLOBALS["default.db.user"] = "root";
     GLOBALS["default.db.password"] = "mysql";

     /**
     engine_.add_libraryload("pf_plugin_odbc", {kDBTypeODBC});

     engine_.init();

     //Normal.
     auto connection = new pf_db::Connection(engine_.get_db());
     unique_move(pf_db::Connection, connection, connection_);
     **/
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
   //static pf_engine::Kernel engine_;
   static std::unique_ptr<pf_db::Connection> connection_;

};

//pf_engine::Kernel DBSchemaBlueprint::engine_;
std::unique_ptr<pf_db::Connection> DBSchemaBlueprint::connection_{nullptr};


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

}
