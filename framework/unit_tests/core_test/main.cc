#include "gtest/gtest.h"
#include "env.h"
#include "plain/net/socket/api.h"

// std::unique_ptr<plain_engine::Kernel> engine{nullptr};

class AllEnvironment : public testing::Environment {

 public:
   virtual void SetUp() {
      assert(plain::net::socket::initialize());
     //std::cout << "SetUp" << std::endl;
   }
   virtual void TearDown() {
     //std::cout << "TearDown" << std::endl;
   }

 /*
 protected:

   std::unique_ptr<plain_engine::Application> app_;
 */

};

int32_t main(int32_t argc, char **argv) {
  /**
  plain::Kernel engine;
  plain::Application app(&engine);
  app.run(argc, argv);
  std::cout << "main" << std::endl;
  **/

  testing::AddGlobalTestEnvironment(new AllEnvironment);
  testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
