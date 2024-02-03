#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TSocket : public testing::Test {

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
  
  void test_net_socket_construct();
  void test_net_socket_operator();
  void test_net_socket_funcs();

}

void plain::tests::test_net_socket_construct() {
  socket::Basic sock;
  socket::Basic sock1{9527};
  socket::Basic sock2{std::move(sock1)};
}

void plain::tests::test_net_socket_operator() {
  socket::Basic sock;
  ASSERT_FALSE(static_cast<bool>(sock));
  socket::Basic sock1{9527};
  ASSERT_TRUE(static_cast<bool>(sock1));
}

void plain::tests::test_net_socket_funcs() {
  using namespace std::chrono_literals;
  socket::Basic sock;
  auto r = sock.create();
  ASSERT_TRUE(r);
  r = sock.create(AF_INET, SOCK_STREAM, 0);
  ASSERT_TRUE(r);
  r = sock.create(AF_INET6, SOCK_STREAM, 0);
  ASSERT_TRUE(r);
  r = sock.create(AF_INET, SOCK_DGRAM, 0);
  ASSERT_TRUE(r);
  r = sock.create(AF_INET6, SOCK_STREAM, 0);
  ASSERT_TRUE(r);
#if OS_UNIX
  r = sock.create(AF_UNIX, SOCK_STREAM, 0);
  ASSERT_TRUE(r);
  r = sock.create(AF_UNIX, SOCK_DGRAM, 0);
  ASSERT_TRUE(r);
#endif

  r = sock.close();
  ASSERT_TRUE(r);

  sock.create();
  auto id = sock.release();
  ASSERT_TRUE(id != socket::kInvalidId);
#if OS_UNIX
  constexpr uint32_t kSendFlag{MSG_NOSIGNAL};
#else
  constexpr uint32_t kSendFlag{0};
#endif
  sock.create();
  auto sock1 = sock.clone();
  ASSERT_TRUE(static_cast<bool>(sock1));
  ASSERT_FALSE(sock.error());
  r = sock.set_nonblocking();
  ASSERT_TRUE(r);
  bytes_t data{reinterpret_cast<const std::byte *>("hello world"), 11};
  auto send_count = sock.send(data, kSendFlag);
  ASSERT_EQ(send_count, socket::kSocketError);
  data.clear();
  data.reserve(128);
  auto recv_count = sock.recv(data);
  ASSERT_EQ(recv_count, socket::kSocketError);

  auto avail_count = sock.avail();
  ASSERT_EQ(avail_count, 0);

  id = sock.id();
  ASSERT_NE(id, socket::kInvalidId);
  const uint32_t size{512};
  r = sock.set_recv_size(size);
  ASSERT_TRUE(r);
  auto r_size = sock.get_recv_size();
  ASSERT_GE(r_size, 0);
  // ASSERT_EQ(size, r_size);
  r = sock.set_send_size(size);
  ASSERT_TRUE(r);
  r_size = sock.get_send_size();
  // ASSERT_EQ(size, r_size);

  r = sock.set_id(9527);
  ASSERT_TRUE(r);

  sock.create();
  r = sock.shutdown();
  ASSERT_TRUE(r);

  r = sock.is_nonblocking();
  ASSERT_FALSE(r);

  sock.create();
  r = sock.bind(Address{":9527"});
  ASSERT_TRUE(r);

  sock.create();
  r = sock.bind();
  ASSERT_FALSE(r);

  auto linger = sock.get_linger();
  std::cout << "linger: " << linger << std::endl;
  r = sock.set_linger(1);
  ASSERT_TRUE(r);
  r = sock.set_reuse_addr();
  ASSERT_TRUE(r);
  r = sock.set_reuse_addr(false);
  ASSERT_TRUE(r);
  r = sock.listen(0);
  ASSERT_TRUE(r);

  sock.set_nonblocking();
  Address addr;
  auto i_r = sock.accept(addr);
  ASSERT_EQ(i_r, socket::kSocketError);
  i_r = sock.accept();
  ASSERT_EQ(i_r, socket::kSocketError);

  sock.create();
  r = sock.connect({":22"});
  ASSERT_TRUE(r);
  sock.create();
  r = sock.connect("127.0.0.1", 22);
  ASSERT_TRUE(r);
  sock.create();
  r = sock.connect({":888"}, 20ms);
  ASSERT_FALSE(r);
  r = sock.connect("127.0.0.1", 888, 20ms);
  ASSERT_FALSE(r);

}

using namespace plain::tests;

TEST_F(TSocket, testConstructor) {
  test_net_socket_construct();
}

TEST_F(TSocket, testOperator) {
  test_net_socket_operator();
}

TEST_F(TSocket, testFunc) {
  test_net_socket_funcs();
}
