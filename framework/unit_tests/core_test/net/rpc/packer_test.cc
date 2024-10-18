#include "gtest/gtest.h"
#include "plain/all.h"
#include "assertions.h"

using namespace plain::net;

class RpcPacker : public testing::Test {

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

struct MapData {
  std::map<std::string, bool> map;

  template <typename T>
  void pack(T &_pack) {
    _pack(map);
  }
};

struct NestedObject {
  std::string nested_value{};

  template<class T>
  void pack(T &pack) {
    pack(nested_value);
  }
};

struct BaseObject {
  int first_member{};
  NestedObject second_member{};

  template<class T>
  void pack(T &pack) {
    pack(first_member, second_member);
  }
};

namespace plain::tests {
  
  void test_net_rpc_packer_construct();
  void test_net_rpc_packer_operator();
  void test_net_rpc_packer_funcs();

  void test_net_rpc_unpacker_construct();
  void test_net_rpc_unpacker_operator();
  void test_net_rpc_unpacker_funcs();

}

void plain::tests::test_net_rpc_packer_construct() {

}

void plain::tests::test_net_rpc_packer_operator() {

}

void plain::tests::test_net_rpc_packer_funcs() {

  MapData example{{{"compact", true}, {"schema", false}}};
  auto data = rpc::pack(example);
  ASSERT_TRUE(data.size() == 18);

  auto temp = std::vector<uint8_t>{
    0x82, 0xa7, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63, 0x74, 11, 0xa6, 0x73,
    0x63, 0x68, 0x65, 0x6d, 0x61, 12};
  ASSERT_TRUE(data == temp);
  auto unpacked = rpc::unpack<MapData>(data).map;
  ASSERT_TRUE(example.map == rpc::unpack<MapData>(data).map);

  auto temp1 = std::vector<uint8_t>{
    0x82, 0xa7, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63, 0x74, 11, 0xa6, 0x73,
    0x63};
  plain::Error e{};
  auto unpacked_object = rpc::unpack<MapData>(temp1, e);
  ASSERT_TRUE(e && e == plain::ErrorCode::OutOfRange);
}

void plain::tests::test_net_rpc_unpacker_construct() {

}

void plain::tests::test_net_rpc_unpacker_operator() {

}

void plain::tests::test_net_rpc_unpacker_funcs() {

  auto object = BaseObject{12345, {"NestedObject"}};
  auto data = rpc::pack(object);
  auto unpacked_object = rpc::unpack<BaseObject>(data);

  ASSERT_TRUE(object.first_member == unpacked_object.first_member);
  auto checked = object.second_member.nested_value ==
    unpacked_object.second_member.nested_value;
  ASSERT_TRUE(checked);
}

using namespace plain::tests;

TEST_F(RpcPacker, testConstructor) {
  test_net_rpc_packer_construct();
}

TEST_F(RpcPacker, testOperator) {
  test_net_rpc_packer_operator();
}

TEST_F(RpcPacker, testFunc) {
  test_net_rpc_packer_funcs();
}

TEST_F(RpcPacker, testUnConstructor) {
  test_net_rpc_unpacker_construct();
}

TEST_F(RpcPacker, testUnOperator) {
  test_net_rpc_unpacker_operator();
}

TEST_F(RpcPacker, testUnFunc) {
  test_net_rpc_unpacker_funcs();
}

using plain::net::rpc::Packer;
using plain::net::rpc::Unpacker;

TEST_F(RpcPacker, NullTypePacking) {
  auto null_obj = std::nullptr_t{};
  auto packer = Packer{};
  packer.process(null_obj);
  ASSERT_TRUE(packer.vector() == std::vector<uint8_t>{0x0});
}

TEST_F(RpcPacker, BooleanTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};
  auto bool_obj = false;
  packer.process(bool_obj);
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  bool_obj = true;
  unpacker.process(bool_obj);
  ASSERT_TRUE(packer.vector() == std::vector<uint8_t>{0xc});
  ASSERT_TRUE(!bool_obj);

  bool_obj = true;
  packer.clear();
  packer.process(bool_obj);
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  bool_obj = false;
  unpacker.process(bool_obj);
  ASSERT_TRUE(packer.vector() == std::vector<uint8_t>{0xb});
  ASSERT_TRUE(bool_obj);
}

TEST_F(RpcPacker, IntegerTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  for (auto i = 0U; i < 10; ++i) {
    uint8_t test_num = i * (std::numeric_limits<uint8_t>::max() / 10);
    packer.clear();
    packer.process(test_num);
    uint8_t x = 0U;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = 0U; i < 10; ++i) {
    uint16_t test_num = i * (std::numeric_limits<uint16_t>::max() / 10);
    packer.clear();
    packer.process(test_num);
    uint16_t x = 0U;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = 0U; i < 10; ++i) {
    uint32_t test_num = i * (std::numeric_limits<uint32_t>::max() / 10);
    packer.clear();
    packer.process(test_num);
    uint32_t x = 0U;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    if (x != test_num)
      std::cout << "test_num: " << test_num << " x: " << x << std::endl;
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = 0U; i < 10; ++i) {
    uint64_t test_num = i * (std::numeric_limits<uint64_t>::max() / 10);
    packer.clear();
    packer.process(test_num);
    uint64_t x = 0U;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = -5; i < 5; ++i) {
    int8_t test_num = i * (std::numeric_limits<int8_t>::max() / 5);
    packer.clear();
    packer.process(test_num);
    int8_t x = 0;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = -5; i < 5; ++i) {
    int16_t test_num = i * (std::numeric_limits<int16_t>::max() / 5);
    packer.clear();
    packer.process(test_num);
    int16_t x = 0;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = -5; i < 5; ++i) {
    int32_t test_num = i * (std::numeric_limits<int32_t>::max() / 5);
    packer.clear();
    packer.process(test_num);
    int32_t x = 0;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = -5; i < 5; ++i) {
    int64_t test_num = i * (std::numeric_limits<int64_t>::max() / 5);
    packer.clear();
    packer.process(test_num);
    int64_t x = 0;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }
}

TEST_F(RpcPacker, ChronoTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto test_time_point = std::chrono::steady_clock::now();
  auto test_time_point_copy = test_time_point;

  packer.process(test_time_point);
  test_time_point = std::chrono::steady_clock::time_point{};
  ASSERT_TRUE(test_time_point != test_time_point_copy);
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(test_time_point);
  ASSERT_TRUE(test_time_point == test_time_point_copy);
}

TEST_F(RpcPacker, FloatTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  for (auto i = -5; i < 5; ++i) {
    float test_num = 5.0f + float(i) * 12345.67f / 4.56f;
    packer.clear();
    packer.process(test_num);
    float x = 0.0f;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }

  for (auto i = -5; i < 5; ++i) {
    double test_num = 5.0 + double(i) * 12345.67 / 4.56;
    packer.clear();
    packer.process(test_num);
    double x = 0.0;
    unpacker.set_data(packer.vector().data(), packer.vector().size());
    unpacker.process(x);
    ASSERT_TRUE(x == test_num);
  }
}

TEST_F(RpcPacker, StringTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto str1 = std::string("test");
  packer.process(str1);
  str1 = "";
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(str1);
  auto temp = std::vector<uint8_t>{0b10100000 | 4, 't', 'e', 's', 't'};
  ASSERT_TRUE(packer.vector() == temp);
  ASSERT_TRUE(str1 == "test");
}

TEST_F(RpcPacker, ByteArrayTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto vec1 = std::vector<uint8_t>{1, 2, 3, 4};
  packer.process(vec1);
  vec1.clear();
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(vec1);
  ASSERT_TRUE((packer.vector() == std::vector<uint8_t>{0xd, 4, 1, 2, 3, 4}));
  ASSERT_TRUE((vec1 == std::vector<uint8_t>{1, 2, 3, 4}));
}

TEST_F(RpcPacker, ArrayTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto list1 = std::list<std::string>{"one", "two", "three"};
  packer.process(list1);
  list1.clear();
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(list1);
  auto temp = std::vector<uint8_t>{0b10010000 | 3, 0b10100000 | 3, 'o', 'n', 'e',
    0b10100000 | 3, 't', 'w', 'o', 0b10100000 | 5, 't', 'h', 'r', 'e', 'e'};
  ASSERT_TRUE(packer.vector() == temp);
  ASSERT_TRUE((list1 == std::list<std::string>{"one", "two", "three"}));
}

TEST_F(RpcPacker, StdArrayTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto arr = std::array<std::string, 3>{"one", "two", "three"};
  packer.process(arr);
  arr.fill("");
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(arr);
  auto temp = std::vector<uint8_t>{0b10010000 | 3, 0b10100000 | 3, 'o', 'n', 'e',
    0b10100000 | 3, 't', 'w', 'o', 0b10100000 | 5, 't', 'h', 'r', 'e', 'e'};
  ASSERT_TRUE(packer.vector() == temp);
  ASSERT_TRUE((arr == std::array<std::string, 3>{"one", "two", "three"}));
}

TEST_F(RpcPacker, MapTypePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto map1 = std::map<uint8_t, std::string>{
    std::make_pair(0, "zero"), std::make_pair(1, "one")};
  packer.process(map1);
  map1.clear();
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(map1);
  auto temp = std::vector<uint8_t>{0b10000000 | 2, 0, 0b10100000 | 4, 'z', 'e',
    'r', 'o', 1, 0b10100000 | 3, 'o', 'n', 'e'};
  ASSERT_TRUE(packer.vector() == temp);
  auto temp1 = std::map<uint8_t, std::string>{
    std::make_pair(0, "zero"), std::make_pair(1, "one")};
  ASSERT_TRUE(map1 == temp1);
}

TEST_F(RpcPacker, UnorderedMapPacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto map1 = std::unordered_map<uint8_t, std::string>{
    std::make_pair(0, "zero"), std::make_pair(1, "one")};
  auto map_copy = map1;
  packer.process(map1);
  map1.clear();
  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(map1);
  ASSERT_TRUE(map1[0] == map_copy[0]);
  ASSERT_TRUE(map1[1] == map_copy[1]);
}

TEST_F(RpcPacker, TuplePacking) {
  auto packer = Packer{};
  auto unpacker = Unpacker{};

  auto tuple1 = std::tuple<int32_t, int32_t>(1, 2);
  auto tuple_copy = tuple1;
  packer.process(tuple1);

  /*
  for (int8_t i = 1; i < std::numeric_limits<int8_t>::max(); ++i) {
    if (uint8_t(packer.twos_complement(i).to_ulong()) <= 32) {
      std::cout << "i: " << int32_t(i) << std::endl;
    }
    if (uint8_t(packer.twos_complement(-i).to_ulong()) > 0) {
      std::cout << "-i: " << int32_t(-i) << "|"
        << uint32_t(uint8_t(packer.twos_complement(-i).to_ulong())) << std::endl;
    }
  }
  */

  unpacker.set_data(packer.vector().data(), packer.vector().size());
  unpacker.process(tuple1);
  std::cout << std::get<0>(tuple1) << "|" << std::get<1>(tuple1) << std::endl;
  std::cout << std::get<0>(tuple_copy) << "|" <<
    std::get<1>(tuple_copy) << std::endl;
  ASSERT_TRUE(tuple1 == tuple_copy);
}
