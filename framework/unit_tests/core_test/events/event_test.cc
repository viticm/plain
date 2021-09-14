#include "gtest/gtest.h"
#include "pf/events/bus.h"

using namespace pf_events;

struct SimpleEvent {
  std::thread::id id;
};

class Event : public testing::Test {

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


static void assertEquals(
    const std::vector<std::string> &a, 
    const std::vector<std::string> &b, 
    int32_t line = -1) {
  if (line != -1)
    std::cout << "assertEquals: " << line << std::endl;
  ASSERT_TRUE(a.size() == b.size());
  for (size_t i = 0; i < a.size(); ++i)
    ASSERT_STREQ(a[i].c_str(), b[i].c_str());
}

constexpr auto ns2 = std::chrono::nanoseconds{2};
constexpr auto ns3 = std::chrono::nanoseconds{3};
/**
static void print_r(const std::vector<std::string> &array) {
  for (auto &str : array)
    std::cout << "str: " << str << std::endl;
}
**/

TEST_F(Event, testEvent) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  std::atomic<int32_t> counter{0};
  listener->listen([&counter](const SimpleEvent &event) { 
    std::cout << "Event from: " << event.id << std::endl;
    ++counter;
  });
  std::thread worker1{[&bus]() { 
    for (int32_t i = 0; i < 10; ++i) {
      bus.postpone(SimpleEvent{std::this_thread::get_id()});
      std::this_thread::sleep_for(ns3);
    }
  }};
  std::thread worker2{[&bus]() {
    for (int32_t i = 0; i < 10; ++i) {
      bus.postpone(SimpleEvent{std::this_thread::get_id()});
      std::this_thread::sleep_for(ns2);
    }
  }};
  ASSERT_TRUE(0 == counter);
  int32_t sum_of_consumed = 0;
  while (counter < 20) {
    ASSERT_TRUE(counter == sum_of_consumed);
    sum_of_consumed += bus.process();
    ASSERT_TRUE(counter == sum_of_consumed);
  }
  worker1.join();
  worker2.join();
}
