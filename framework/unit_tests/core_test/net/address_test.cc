#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TAddress : public testing::Test {

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

void test_net_addr_constructor();
void test_net_addr_operator();
void test_net_addr_func();

}

void plain::tests::test_net_addr_constructor() {
  assert_throws<std::invalid_argument>([] {
    Address addr{"x"};
  });
  assert_throws<std::invalid_argument>([] {
    Address addr{"x:"};
  });
  assert_throws<std::invalid_argument>([] {
    Address addr{"x:x"};
  });
  /*
  assert_throws<std::invalid_argument>([] {
    Address addr{"::"};
  });
  */
  assert_throws<std::invalid_argument>([] {
    Address addr{"[xx.xx.xx]:"};
  });
  Address addr_ip_v4_1{":80"};
  Address addr_ip_v4_2{"127.0.0.1:80"};
  Address addr_ip_v4_3{"127.0.0.1:80", false};
  Address addr_ip_v6_1{"[::]:80"};
  Address addr_ip_v6_2{"[0:0:0:0:0:0:0:0]:80"};
  Address addr_ip_v6_3{"[2409:8c20:1833:2000::afd:b739]:80", false};
}

void plain::tests::test_net_addr_operator() {
  Address addr1{};
  ASSERT_FALSE(static_cast<bool>(addr1));

  Address addr2{":80"};
  ASSERT_TRUE(static_cast<bool>(addr2));
}

void plain::tests::test_net_addr_func() {
  Address addr_ip_v4_1{":80"};
  Address addr_ip_v4_2{"127.0.0.1:80"};
  Address addr_ip_v4_3{"127.0.0.1:80", false};
  Address addr_ip_v6_1{"[::]:80"};
  Address addr_ip_v6_2{"[0:0:0:0:0:0:0:0]:80"};
  Address addr_ip_v6_3{"[2409:8c20:1833:2000::afd:b739]:80", false};

  ASSERT_EQ(addr_ip_v4_1.family(), AF_INET);
  ASSERT_EQ(addr_ip_v4_2.family(), AF_INET);
  ASSERT_EQ(addr_ip_v4_3.family(), AF_INET);
  ASSERT_EQ(addr_ip_v6_1.family(), AF_INET6);
  ASSERT_EQ(addr_ip_v6_2.family(), AF_INET6);
  ASSERT_EQ(addr_ip_v6_3.family(), AF_INET6);

  ASSERT_EQ(addr_ip_v4_1.port(), 80);
  ASSERT_EQ(addr_ip_v4_2.port(), 80);
  ASSERT_EQ(addr_ip_v4_3.port(), 80);
  ASSERT_EQ(addr_ip_v6_1.port(), 80);
  ASSERT_EQ(addr_ip_v6_2.port(), 80);
  ASSERT_EQ(addr_ip_v6_3.port(), 80);

  ASSERT_EQ(addr_ip_v4_1.size(), sizeof(sockaddr_in));
  ASSERT_EQ(addr_ip_v4_2.size(), sizeof(sockaddr_in));
  ASSERT_EQ(addr_ip_v4_3.size(), sizeof(sockaddr_in));
  ASSERT_EQ(addr_ip_v6_1.size(), sizeof(sockaddr_in6));
  ASSERT_EQ(addr_ip_v6_2.size(), sizeof(sockaddr_in6));
  ASSERT_EQ(addr_ip_v6_3.size(), sizeof(sockaddr_in6));

  /*
  std::cout << addr_ip_v4_1.text() << std::endl;
  std::cout << addr_ip_v4_2.text() << std::endl;
  std::cout << addr_ip_v4_3.text() << std::endl;
  std::cout << addr_ip_v6_1.text() << std::endl;
  std::cout << addr_ip_v6_2.text() << std::endl;
  std::cout << addr_ip_v6_3.text() << std::endl;
  */
}

using namespace plain::tests;

TEST_F(TAddress, testConstructor) {
  test_net_addr_constructor();
}

TEST_F(TAddress, testOperator) {
  test_net_addr_operator();
}

TEST_F(TAddress, testFunc) {
  test_net_addr_func();
}
