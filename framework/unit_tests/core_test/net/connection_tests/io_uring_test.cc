#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TIoUring : public testing::Test {

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
  
  void test_net_io_uring_construct();
  void test_net_io_uring_operator();
  void test_net_io_uring_funcs();

}

void plain::tests::test_net_io_uring_construct() {
  setting_t setting;
  setting.mode = Mode::IoUring;
  auto e1 = std::make_shared<connection::IoUring>(setting);
  auto e2 = std::make_shared<connection::IoUring>(
    setting, std::make_shared<concurrency::executor::WorkerThread>());
}

void plain::tests::test_net_io_uring_operator() {

}

void plain::tests::test_net_io_uring_funcs() {

}

using namespace plain::tests;

TEST_F(TIoUring, testConstructor) {
  test_net_io_uring_construct();
}
