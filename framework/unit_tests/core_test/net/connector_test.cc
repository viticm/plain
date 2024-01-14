#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TConnector : public testing::Test {

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

void test_net_connector_constructor();
void test_net_conncetor_operator();
void test_net_connector_func();

}

void plain::tests::test_net_connector_constructor() {
  Connector connector1;
  setting_t setting;
  setting.max_count = 128;
  setting.default_count = 32;
  Connector connector2(setting);
  Connector connector3{
    std::make_unique<plain::concurrency::executor::Thread>()};
  Connector connector4{
    std::make_unique<plain::concurrency::executor::ThreadPool>(
      "test", 2, std::chrono::seconds(10))};
  Connector connector5{
    std::make_unique<plain::concurrency::executor::ThreadPool>(
      "test1", 2, std::chrono::seconds(10)), setting};
}

void plain::tests::test_net_connector_func() {
  Connector connector1;
  auto conn = connector1.connect("127.0.0.1", 80);
  ASSERT_TRUE(static_cast<bool>(conn));
}

using namespace plain::tests;

TEST_F(TConnector, testConstructor) {
  test_net_connector_constructor();
}

TEST_F(TConnector, testFunc) {
  test_net_connector_func();
}
