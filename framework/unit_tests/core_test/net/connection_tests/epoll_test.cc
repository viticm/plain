#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TEpoll : public testing::Test {

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
  
  void test_net_epoll_construct();
  void test_net_epoll_operator();
  void test_net_epoll_funcs();

}

void plain::tests::test_net_epoll_construct() {
  setting_t setting;
  setting.mode = Mode::Epoll;
  auto e1 = std::make_shared<connection::Epoll>(setting);
  auto e2 = std::make_shared<connection::Epoll>(
    setting, std::make_shared<concurrency::executor::WorkerThread>());
}

void plain::tests::test_net_epoll_operator() {

}

void plain::tests::test_net_epoll_funcs() {

}

using namespace plain::tests;

TEST_F(TEpoll, testConstructor) {
  test_net_epoll_construct();
}
