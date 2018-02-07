#include "gtest/gtest.h"
#include "pf/all.h"

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
  testing::AddGlobalTestEnvironment(new AllEnvironment);
  testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
