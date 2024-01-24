#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TListener : public testing::Test {

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

void test_net_listener_constructor();
void test_net_listener_operator();
void test_net_listener_func();

}

void plain::tests::test_net_listener_constructor() {
  Listener listener1;
  setting_t setting;
  setting.max_count = 128;
  setting.default_count = 32;
  Listener listener2(setting);
  Listener listener3{
    std::make_unique<plain::concurrency::executor::Thread>()};
  Listener listener4{
    std::make_unique<plain::concurrency::executor::ThreadPool>(
      "test", 2, std::chrono::seconds(10))};
  Listener listener5{
    std::make_unique<plain::concurrency::executor::ThreadPool>(
      "test1", 2, std::chrono::seconds(10)), setting};
}

void plain::tests::test_net_listener_operator() {

}

void plain::tests::test_net_listener_func() {
  using namespace std::chrono_literals;
  Listener listener1;
  auto started = listener1.start();
  ASSERT_TRUE(started);
  auto addr1 = listener1.address();
  ASSERT_GT(addr1.port(), 0);

  setting_t setting;
  setting.mode = Mode::Epoll;
  setting.address = ":9527";
  setting.name = "listener2";
  Listener listener2(setting);
  started = listener2.start();
  Connector connector;
  auto started1 = connector.start();
  ASSERT_TRUE(started1);
  auto pack = std::make_shared<packet::Basic>();
  pack->set_id(1);
  pack->set_writeable(true);
  // std::cout << "pack size: " << pack->data().size() << std::endl;
  *pack << std::string{"hello"} << " world";

#if OS_UNIX
  ASSERT_TRUE(started);
  auto conn = connector.connect(":9527");
  ASSERT_TRUE(static_cast<bool>(conn));
  conn->send(pack);
  std::this_thread::sleep_for(10ms);
#elif OS_WIN
 ASSERT_FALSE(started);
#endif

  setting.address = "[::]:9528";
  setting.name = "listener3";
  Listener listener3(setting);
  started = listener3.start();
#if OS_UNIX
  ASSERT_TRUE(started);
  auto conn1 = connector.connect("[::]:9528");
  ASSERT_TRUE(static_cast<bool>(conn1));
  conn1->send(pack);
  std::this_thread::sleep_for(10ms);
#elif OS_WIN
  ASSERT_FALSE(started);
#endif

  setting.address = "[::]:9529";
  setting.name = "listener4";
  Listener listener4(setting);
  started = listener4.start();
  ASSERT_TRUE(started);
  listener4.set_dispatcher([](
    connection::Basic *conn, std::shared_ptr<packet::Basic> packet) {
    std::string value1;
    std::string value2;
    *packet >> value1 >> value2;
    std::cout << "handle: " << conn->id() << "|" << packet->id()
      << "|" << value1 << value2 << std::endl;
    return true;
  });
  auto conn2 = connector.connect("[::]:9529");
  ASSERT_TRUE(conn2);
  conn2->send(pack);
  std::this_thread::sleep_for(50ms);
 
}

using namespace plain::tests;

TEST_F(TListener, testConstructor) {
  test_net_listener_constructor();
}

TEST_F(TListener, testFunc) {
  test_net_listener_func();
}
