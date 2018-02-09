#include "gtest/gtest.h"
#include "pf/engine/kernel.h"
#include "pf/db/query/builder.h"

enum {
  kDBTypeODBC = 1,
};

using namespace pf_db::query;

class DBQueryBuilder : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //GLOBALS["log.print"] = false; //First forbid the log print.

     GLOBALS["default.db.open"] = true;
     GLOBALS["default.db.type"] = kDBTypeODBC;
     GLOBALS["default.db.name"] = "pf_test";
     GLOBALS["default.db.user"] = "root";
     GLOBALS["default.db.password"] = "mysql";

     engine_.add_libraryload("pf_plugin_odbc", {kDBTypeODBC});

     engine_.init();
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

};

pf_engine::Kernel DBQueryBuilder::engine_;

TEST_F(DBQueryBuilder, Init) {
  Builder object(nullptr, nullptr);
}

TEST_F(DBQueryBuilder, function) {

}
