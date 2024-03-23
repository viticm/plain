#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TStream : public testing::Test {

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
  
  void test_net_stream_construct();
  void test_net_stream_operator();
  void test_net_stream_funcs();

}

void plain::tests::test_net_stream_construct() {
  auto sock = std::make_shared<socket::Basic>();
  auto stream = stream::Basic{sock};
}

void plain::tests::test_net_stream_funcs() {
  auto sock = std::make_shared<socket::Basic>();
  auto stream = stream::Basic{sock};
  
  auto r = stream.pull();
  ASSERT_EQ(r, 0);
  r = stream.push();
  ASSERT_EQ(r, 0);

  r = stream.full();
  ASSERT_FALSE(r);
  r = stream.empty();
  ASSERT_TRUE(r);
  auto r_size = stream.size();
  ASSERT_EQ(r_size, 0);
  ASSERT_TRUE(static_cast<bool>(stream.socket()));

  r_size = stream.write("hello world");
  ASSERT_EQ(r_size, 11);
  bytes_t bytes{reinterpret_cast<const std::byte *>("hello world"), 11};
  r_size = stream.write(bytes);
  ASSERT_EQ(r_size, 11);
  r_size = stream.write(reinterpret_cast<const std::byte *>("hello world"));
  ASSERT_EQ(r_size, 11);

  std::string str;
  str.resize(11);
  r_size = stream.read(str);
  ASSERT_EQ(r_size, 11);
  bytes_t r_bytes;
  r_bytes.resize(11);
  r_size = stream.read(r_bytes);
  ASSERT_EQ(r_size, 11);
  std::array<std::byte, 32> array;
  r_size = stream.read(array.data(), 11);
  ASSERT_EQ(r_size, 11);
}

using namespace plain::tests;

TEST_F(TStream, testConstructor) {
  test_net_stream_construct();
}

TEST_F(TStream, testFunc) {
  test_net_stream_funcs();
}
