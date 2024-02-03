#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TIocp : public testing::Test {

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
  
  void test_net_iocp_construct();
  void test_net_iocp_operator();
  void test_net_iocp_funcs();

}

void plain::tests::test_net_iocp_construct() {
  setting_t setting;
  setting.mode = Mode::Iocp;
  auto e1 = std::make_shared<connection::Iocp>(setting);
  auto e2 = std::make_shared<connection::Iocp>(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
}

void plain::tests::test_net_iocp_operator() {

}

void plain::tests::test_net_iocp_funcs() {

}

using namespace plain::tests;

TEST_F(TIocp, testConstructor) {
  test_net_iocp_construct();
}
