#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class TPacket : public testing::Test {

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

void test_net_packet_constructor();
void test_net_packet_operator();
void test_net_packet_func();

}

void plain::tests::test_net_packet_constructor() {
  packet::Basic pack;
}

void plain::tests::test_net_packet_operator() {

}

void plain::tests::test_net_packet_func() {
  packet::Basic pack;
  pack.set_id(1);
  ASSERT_EQ(pack.id(), 1);

  auto r = pack.write("hello world");
  ASSERT_EQ(r, 0);
  r = pack.write(reinterpret_cast<const std::byte *>("hello world"));
  ASSERT_EQ(r, 0);
  r = pack.write(reinterpret_cast<const std::byte *>("hello world"), 11);
  ASSERT_EQ(r, 0);

  pack.set_writeable(true);

  r = pack.write("hello world");
  ASSERT_EQ(r, 11);
  r = pack.write(reinterpret_cast<const std::byte *>("hello world"));
  ASSERT_EQ(r, 11);
  r = pack.write(reinterpret_cast<const std::byte *>("hello world"), 11);
  ASSERT_EQ(r, 11);

  std::string str;
  bytes_t bytes;
  std::array<std::byte, 64> array;
  r = pack.read(str);
  ASSERT_EQ(str.size(), 0);
  r = pack.read(bytes);
  ASSERT_EQ(bytes.size(), 0);
  r = pack.read(array.data(), array.size());
  ASSERT_EQ(r, 0);

  pack.set_readable(true);
  str.clear();
  r = pack.read(str);
  ASSERT_EQ(str.size(), 11);
  ASSERT_EQ(str, "hello world");
  bytes.clear();
  r = pack.read(bytes);
  ASSERT_EQ(str.size(), 11);
  ASSERT_EQ(as_string_view(bytes), "hello world");
  std::memset(array.data(), 0, array.size());
  r = pack.read(array.data(), 11);
  ASSERT_EQ(r, 11);
  ASSERT_EQ(
    std::string_view(reinterpret_cast<char *>(array.data()), r), "hello world");

  bytes_t temp{reinterpret_cast<const std::byte *>("hello world"), 11};
  pack << "hello world" << temp << 11 << 11.16f << 11.16;
  str.clear();
  bytes.clear();
  int32_t value_i{0};
  float value_f{0.f};
  double value_d{0.0};
  pack >> str >> bytes >> value_i >> value_f >> value_d;
  ASSERT_EQ(str, "hello world");
  ASSERT_EQ(as_string_view(bytes), "hello world");
  ASSERT_EQ(value_i, 11);
  ASSERT_EQ(value_f, 11.16f);
  ASSERT_EQ(value_d, 11.16);

  pack << "hello world";
  auto remove_size = pack.remove(11);
  ASSERT_EQ(remove_size, 11);
  str.clear();
  pack >> str;
  ASSERT_TRUE(str.empty());
  std::cout << "pack.size: " << pack.data().size() << std::endl;

}

using namespace plain::tests;

TEST_F(TPacket, testConstructor) {
  test_net_packet_constructor();
}

TEST_F(TPacket, testFunc) {
  test_net_packet_func();
}
