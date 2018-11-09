#include "gtest/gtest.h"
#include "env.h"
#include "pf/all.h"

std::unique_ptr<pf_engine::Kernel> engine{nullptr};

class AllEnvironment : public testing::Environment {

 public:
   virtual void SetUp() {
     //std::cout << "SetUp" << std::endl;
   }
   virtual void TearDown() {
     //std::cout << "TearDown" << std::endl;
   }

 protected:

   std::unique_ptr<pf_engine::Application> app_;

};

int32_t main(int32_t argc, char **argv) {
  /**
  pf_engine::Kernel engine;
  pf_engine::Application app(&engine);
  app.run(argc, argv);
  std::cout << "main" << std::endl;
  **/

  GLOBALS["log.print"] = false;
  GLOBALS["default.db.open"] = true;
  GLOBALS["default.db.type"] = kDBTypeNull;
  GLOBALS["default.db.name"] = "pf_test";
  GLOBALS["default.db.user"] = "root";
  GLOBALS["default.db.password"] = "mysql";
  auto _engine = new pf_engine::Kernel;
  unique_move(pf_engine::Kernel, _engine, engine);

  engine->init();

  testing::AddGlobalTestEnvironment(new AllEnvironment);
  testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
