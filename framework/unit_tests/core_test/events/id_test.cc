#include "gtest/gtest.h"
#include "pf/events/id.h"

namespace {

struct Anonymous {};

} // namespace

struct TestA {
  int a;
};

namespace Test1 {

struct TestA {
  bool b;
};

namespace TestN {

struct TestA {
  long long c;
};

} // namespace TestN

} // namespace Test

using namespace pf_events;

class EventID : public testing::Test {

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

TEST_F(EventID, testNormal) {

	std::set<pf_events::id_t> unique;

  ASSERT_TRUE(unique.insert(get_id<Anonymous>()).second);
  ASSERT_FALSE(unique.insert(get_id<Anonymous>()).second); // already there

  // struct TestA // "name collision" but not quite collision
  // {};

  // ASSERT_TRUE(unique.insert(get_id<TestA>()).second);
  ASSERT_TRUE(unique.insert(get_id<::TestA>()).second);
  ASSERT_TRUE(unique.insert(get_id<Test1::TestA>()).second);
  ASSERT_TRUE(unique.insert(get_id<Test1::TestN::TestA>()).second);
}
