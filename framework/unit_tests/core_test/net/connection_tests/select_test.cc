#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TSelect : public testing::Test {

 public:
  static void SetUpTestCase() {
    //Normal.
  }

  static void TearDownTestCase() {
    //std::cout << "TearDownTestCase" << std::endl;
  }

 public:

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

};

namespace plain::tests {
  
  void test_net_select_construct();
  void test_net_select_operator();
  void test_net_select_funcs();

}

void plain::tests::test_net_select_construct() {
  setting_t setting;
  setting.mode = Mode::Select;
  auto e1 = std::make_shared<connection::Select>(setting);
  auto e2 = std::make_shared<connection::Select>(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
}

void plain::tests::test_net_select_operator() {

}

void plain::tests::test_net_select_funcs() {

}

using namespace plain::tests;

TEST_F(TSelect, testConstructor) {
  test_net_select_construct();
}
