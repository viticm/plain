#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TSockListener : public testing::Test {

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
  
  void test_net_socket_listener_construct();
  void test_net_socket_listener_operator();
  void test_net_socket_listener_funcs();

}

void plain::tests::test_net_socket_listener_construct() {
  socket::Listener listener;
}

void plain::tests::test_net_socket_listener_operator() {

}

void plain::tests::test_net_socket_listener_funcs() {
  socket::Listener listener;
  auto r = listener.init({":9527"});
  ASSERT_TRUE(r);
  auto sock = std::make_shared<socket::Basic>();
  r = listener.accept(sock);
  ASSERT_FALSE(r);
  auto id = listener.id();
  ASSERT_NE(id, socket::kInvalidId);
  auto port = listener.address().port();
  ASSERT_EQ(port, 9527);
}

using namespace plain::tests;

TEST_F(TSockListener, testConstructor) {
  test_net_socket_listener_construct();
}

TEST_F(TSockListener, testFunc) {
  test_net_socket_listener_funcs();
}
