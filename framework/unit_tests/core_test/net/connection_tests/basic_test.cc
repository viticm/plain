#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TConnection : public testing::Test {

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

extern
plain::error_or_t<std::shared_ptr<packet::Basic>>
line_decode(
  stream::Basic *input, const packet::limit_t &packet_limit);

extern
plain::bytes_t line_encode(std::shared_ptr<packet::Basic> packet);

void test_net_connection_construct();
void test_net_connection_operator();
void test_net_connection_funcs();
void test_net_connection_send_line(
  connection::Basic *conn, std::string_view str);

}

void plain::tests::test_net_connection_construct() {
  connection::Basic conn;
  connection::Basic conn1{std::move(conn)};
}

void plain::tests::test_net_connection_operator() {
  connection::Basic conn;
  connection::Basic conn1 = std::move(conn);
}

void plain::tests::test_net_connection_send_line(
  connection::Basic *conn, std::string_view str) {
  assert(conn);
  auto pack = std::make_shared<packet::Basic>();
  auto bytes = as_const_bytes(str);
  pack->set_writeable(true);
  pack->write(bytes.data(), bytes.size());
  //pack->write(reinterpret_cast<const std::byte *>("\n"), 1);
  pack->set_writeable(false);
  auto r = conn->send(pack);
  ASSERT_TRUE(r);
}

void plain::tests::test_net_connection_funcs() {
  using namespace std::chrono_literals;
  
  connection::Basic conn;
  auto r = conn.init();
  ASSERT_TRUE(r);
  r = conn.idle();
  ASSERT_TRUE(r);
  r = conn.shutdown();
  ASSERT_TRUE(r);
  r = conn.valid();
  ASSERT_FALSE(r);
  ASSERT_EQ(conn.id(), connection::kInvalidId);
  ASSERT_TRUE(conn.name().empty());
  ASSERT_TRUE(static_cast<bool>(conn.socket()));

  setting_t setting;
  setting.address = ":9527";
  setting.name = "test";
  Listener listener(setting);
  r = listener.start();
  ASSERT_TRUE(r);
  listener.set_codec({.encode = line_encode, .decode = line_decode});
  listener.set_connect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " connected" << std::endl;
    test_net_connection_send_line(conn, "plain framework");
  });
  listener.set_disconnect_callback([](connection::Basic *conn) {
    std::cout << "disconnect: " << conn->name() << std::endl;
  });
  listener.set_dispatcher(
    [](connection::Basic *conn, std::shared_ptr<packet::Basic> pack) {
    auto d = pack->data();
    std::cout << conn->name() << ": " << as_string_view(d) << std::endl;
    return true;
  });

  listener.bind("sum", [](int32_t a, int32_t b){
    return a * b;
  });

  Connector connector;
  r = connector.start();
  ASSERT_TRUE(r);

  auto init_func = [](connection::Basic *conn) -> bool {
    conn->set_codec({.encode = line_encode, .decode = line_decode});
    conn->set_connect_callback([](connection::Basic *conn) {
      std::cout << "send hello" << std::endl;
      test_net_connection_send_line(conn, "hello world");
    });
    conn->set_disconnect_callback([](connection::Basic *conn) {
      std::cout << "disconnect: " << conn->name() << std::endl;
    });
    conn->set_dispatcher(
      [](connection::Basic *conn, std::shared_ptr<packet::Basic> pack) {
      auto d = pack->data();
      std::cout << conn->name() << ": " << as_string_view(d) << std::endl;
      return true;
    });
    return true;
  };

  auto conn1 = connector.connect(":9527", init_func);
  ASSERT_TRUE(static_cast<bool>(conn1));

  auto conn2 = connector.connect(":9527", nullptr, 5s);
  ASSERT_TRUE(static_cast<bool>(conn2));

  auto call_r0 = conn1->call("sum", 11, 9).as<int32_t>();
  ASSERT_TRUE(call_r0 == (11 * 9));

  // For call.
  setting_t setting1;
  setting1.address = ":9528";
  setting1.name = "test1";
  Listener listener1(setting1);
  listener1.bind("add", [](int32_t a, int32_t b) {
    return a + b;
  });
  listener1.bind("hello", []() {
    return std::string{"world!"};
  });

  r = listener1.start();
  ASSERT_TRUE(r);
  listener1.set_connect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " connected" << std::endl;
  });

  auto conn3 = connector.connect(":9528", nullptr, 5s);
  ASSERT_TRUE(static_cast<bool>(conn3));

  auto call_r = conn3->call("add", 11, 21).as<int32_t>();
  ASSERT_TRUE(call_r == (11 + 21));

  std::cout << "call_r: " << call_r << std::endl;

  auto call_r1 = conn3->call("hello").as<std::string>();
  ASSERT_TRUE(call_r1 == std::string{"world!"});

 
  std::this_thread::sleep_for(50ms);
}

using namespace plain::tests;

TEST_F(TConnection, testConstructor) {
  test_net_connection_construct();
}

TEST_F(TConnection, testOperator) {
  test_net_connection_operator();
}

TEST_F(TConnection, testFunc) {
  test_net_connection_funcs();
}
