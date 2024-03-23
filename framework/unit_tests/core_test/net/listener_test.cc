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

error_or_t<std::shared_ptr<packet::Basic>>
line_decode(stream::Basic *input, const packet::limit_t &packet_limit);

bytes_t line_encode(std::shared_ptr<packet::Basic> packet);

}

void plain::tests::test_net_listener_constructor() {
  Listener listener1;
  setting_t setting;
  setting.max_count = 128;
  setting.default_count = 32;
  Listener listener2(setting);
  Listener listener3{
    setting_t{},
    std::make_shared<plain::concurrency::executor::Thread>()};
  Listener listener4{
    setting_t{},
    std::make_unique<plain::concurrency::executor::ThreadPool>(
      "test", 2, std::chrono::seconds(10))};
  Listener listener5{
    setting_t{},
    std::make_unique<plain::concurrency::executor::ThreadPool>(
      "test1", 2, std::chrono::seconds(10))};
}

void plain::tests::test_net_listener_operator() {

}

plain::error_or_t<std::shared_ptr<packet::Basic>>
plain::tests::line_decode(
  stream::Basic *input, const packet::limit_t &packet_limit) {
  if (!input) return Error{ErrorCode::RunTime};
  bytes_t bytes;
  bytes.reserve(packet_limit.max_length);
  auto readed = input->peek(bytes.data(), bytes.capacity());
  if (readed == 0) return ErrorCode{ErrorCode::NetPacketNeedRecv};
  std::string_view str{reinterpret_cast<char *>(bytes.data()), readed};
  auto pos = str.find('\n');
  if (pos == std::string::npos) {
    if (readed == packet_limit.max_length )
      return Error{ErrorCode::RunTime};
    else
      return Error{ErrorCode::NetPacketNeedRecv};
  }
  input->remove(pos + 1); // remove readed line.
  if (pos > 0 && str[pos - 1] == '\r') {
    pos -= 1;
  }
  auto p = std::make_shared<packet::Basic>();
  if (pos > 0) {
    p->set_writeable(true);
    p->write(bytes.data(), pos);
    p->set_writeable(false);
  }
  p->set_readable(true);
  return p;
}

plain::bytes_t plain::tests::line_encode(std::shared_ptr<packet::Basic> packet) {
  auto d = packet->data();
  return {d.data(), d.size()};
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

  setting.mode = Mode::Select;
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

  setting.address = ":9530";
  setting.name = "listener5";
  Listener listener5(setting);
  started = listener5.start();
  ASSERT_TRUE(started);
  listener5.set_codec({.encode = line_encode, .decode = line_decode});
  listener5.set_dispatcher([](
    connection::Basic *conn, std::shared_ptr<packet::Basic> packet) {
    std::cout << conn->name() << ": " <<
      reinterpret_cast<const char *>(packet->data().data()) << std::endl;
    return true;
  });
  listener5.set_connect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " connected" << std::endl;
  });
  listener5.set_disconnect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " disconnected" << std::endl;
  });

  auto pack1 = std::make_shared<packet::Basic>();
  pack1->set_writeable(true);
  std::string line{"hello world\n"};
  pack1->write(reinterpret_cast<std::byte *>(line.data()), line.size());
  std::string line1{"plain\n"};
  pack1->write(reinterpret_cast<std::byte *>(line1.data()), line1.size());
  auto conn3 = connector.connect(":9530");
  ASSERT_TRUE(conn3);
  conn3->set_codec({.encode = line_encode, .decode = line_decode});
  conn3->send(pack1);
  // conn3->ostream().write("framework\n");

  auto conn4 = connector.connect(":9530");
  ASSERT_TRUE(conn4);
  conn4->close();

  // Io uring | iocp | kqueue
  setting.address = ":9531";
  setting.name = "listener6";
#if OS_WIN
  setting.mode = Mode::Iocp;
#elif OS_MAC
  setting.mode = Mode::Kqueue;
#elif OS_UNIX
  setting.mode = Mode::IoUring;
#endif
  Listener listener6(setting);
  started = listener6.start();
  // ASSERT_TRUE(started);
  listener6.set_codec({.encode = line_encode, .decode = line_decode});
  listener6.set_dispatcher([](
    connection::Basic *conn, std::shared_ptr<packet::Basic> packet) {
    std::cout << conn->name() << ": " <<
      reinterpret_cast<const char *>(packet->data().data()) << std::endl;
    return true;
  });
  listener6.set_connect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " connected" << std::endl;
  });
  listener6.set_disconnect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " disconnected" << std::endl;
  });

  if (started) {
    auto conn5 = connector.connect(":9531");
    ASSERT_TRUE(conn5);
    conn5->set_codec({.encode = line_encode, .decode = line_decode});
    conn5->send(pack1);
    // conn5->close();
  }

  std::this_thread::sleep_for(100ms);
}

using namespace plain::tests;

TEST_F(TListener, testConstructor) {
  test_net_listener_constructor();
}

TEST_F(TListener, testFunc) {
  test_net_listener_func();
}
