#include "event.h"
#include "gtest/gtest.h"
#include "pf/events/bus.h"
#include "pf/events/perk/wait.h"
#include "pf/events/perk/tag.h"
#include "pf/events/perk/bus.h"

using namespace pf_events;

struct SimpleEvent {
  std::thread::id id;
};

class EventConcurrent : public testing::Test {

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

TEST_F(EventConcurrent, testNormal) {
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

TEST_F(EventConcurrent, testOnlyOne) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  std::atomic<int32_t> counter{0};
  listener->listen([&counter, &listener](const SimpleEvent &event) { 
    std::cout << "Event from: " << event.id << std::endl;
    ++counter;
    listener->unlisten_all();
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
  std::thread worker3{[&bus, &counter]() { 
    while (0 == counter) {
      bus.process();
      std::this_thread::sleep_for(ns2);
    }
    for (int32_t i = 0; i < 10; ++i) {
      bus.process();
      std::this_thread::sleep_for(ns2);
    }
  }};
  for (int32_t i = 0; i < 10; ++i) {
    bus.postpone(SimpleEvent{std::this_thread::get_id()});
  }
  worker1.join();
  worker2.join();
  worker3.join();

  // std::cout << "counter: " << counter << std::endl;

  ASSERT_TRUE(1 == counter); // Should be called only once.
}

TEST_F(EventConcurrent, waitWork) {
  perk::Bus bus;
  bus.add_perk(pf_basic::make_unique<perk::Wait>())
    .register_post_postpone(&perk::Wait::on_postpone_event);
  auto *wait_perk = bus.get_perk<perk::Wait>();
  ASSERT_TRUE(wait_perk != nullptr);
  auto listener = Listener<Bus>::create_not_owning(bus);
  listener->listen([](const WaitPerk &) { 
    std::cout << "In WaitPerk event" << std::endl;  
  });
  listener->listen([](const T1 &) { 
    std::cout << "In T1 event" << std::endl;  
  });

  // Worker which will send event every 10 ms
  std::atomic<bool> is_working{true};
  std::atomic<int> produced{0};
  std::atomic<int> consumed{0};

  std::thread producer{[&bus, &is_working, &produced]() { 
    while(is_working) {
      bus.postpone(WaitPerk{});
      bus.postpone(T1{});
      ++produced;
      ++produced;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }};

  for (int32_t i = 0; i < 20; ++i) {
    auto start = std::chrono::steady_clock::now();
    if (wait_perk->wait_for(std::chrono::milliseconds(20))) {
      int32_t before_consumed = consumed;
      consumed += bus.process();
      std::cout << "If events available then consumed count should change" 
        << std::endl;
      EXPECT_TRUE(consumed >= before_consumed);
    }
    std::cout << "I was sleeping for: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start).count()
      << " ms, consumed:" << consumed << std::endl;
  }
  is_working = false;
  producer.join();
  ASSERT_TRUE(produced >= consumed);
}
