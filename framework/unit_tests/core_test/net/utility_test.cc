#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TUtility : public testing::Test {

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

void test_net_utility_funcs();

}

void plain::tests::test_net_utility_funcs() {
  setting_t setting;
  setting.mode = Mode::Select;
  auto m1 = make_manager(setting);
  ASSERT_TRUE(m1);
  setting.mode = Mode::Epoll;
  auto m2 = make_manager(setting);
  ASSERT_TRUE(m2);
  setting.mode = Mode::IoUring;
  auto m3 = make_manager(setting);
  ASSERT_TRUE(m3);
  setting.mode = Mode::Iocp;
  auto m4 = make_manager(setting);
  ASSERT_TRUE(m4);
  setting.mode = Mode::Kqueue;
  auto m5 = make_manager(setting);
  ASSERT_TRUE(m5);

  setting.mode = Mode::Select;
  m1 = make_manager(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
  ASSERT_TRUE(m1);
  setting.mode = Mode::Epoll;
  m2 = make_manager(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
  ASSERT_TRUE(m2);
  setting.mode = Mode::IoUring;
  m3 = make_manager(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
  ASSERT_TRUE(m3);
  setting.mode = Mode::Iocp;
  m4 = make_manager(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
  ASSERT_TRUE(m4);
  setting.mode = Mode::Kqueue;
  m5 = make_manager(
    std::make_unique<concurrency::executor::WorkerThread>(), setting);
  ASSERT_TRUE(m5);

}

using namespace plain::tests;

TEST_F(TUtility, testFunc) {
  test_net_utility_funcs();
}
