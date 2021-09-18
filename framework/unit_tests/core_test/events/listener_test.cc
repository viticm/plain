#include "gtest/gtest.h"
#include "event.h"
#include "pf/events/bus.h"

class EventListener : public testing::Test {

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

using namespace pf_events;

// TEST_CASE("Should remove all 
// listeners When use unlisten_all", "[Bus][Listener]")
TEST_F(EventListener, testRemoveAll){

  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);

  int32_t call_count{0};
  listener->listen([&](const Value &event) {
    ASSERT_TRUE(event.value == 3);
    ++call_count;
  });
  listener->listen([&](const T1 &) { ++call_count; });

  bus.postpone(Value{3});
  bus.postpone(T1{});
  ASSERT_TRUE(bus.process() == 2);
  ASSERT_TRUE(call_count == 2);

  listener->unlisten_all();

  bus.postpone(Value{2});
  bus.postpone(T1{});
  ASSERT_TRUE(bus.process() == 2);
  ASSERT_TRUE(call_count == 2); // unchanged
}

// TEST_CASE("Should unlisten all events When 
// listener instance is overriden", "[Bus][Listener]")
TEST_F(EventListener, testUnlistenAll){
  Bus bus;
  int32_t call_count{0};
  auto listener = Listener<Bus>::create_not_owning(bus);
  listener->listen([&](const Value &event) {
    ASSERT_TRUE(event.value == 3);
    ++call_count;
  });
  bus.postpone(Value{3});
  ASSERT_TRUE(bus.process() == 1);
  ASSERT_TRUE(call_count == 1);

  listener->transfer(Listener<Bus>{});

  bus.postpone(Value{2});
  ASSERT_TRUE(bus.process() == 1);
  ASSERT_TRUE(call_count == 1);
}

// TEST_CASE("Should unlisten all events 
// When listener instance is destroyed", "[Bus][Listener]")
TEST_F(EventListener, testUnlistenAll1) {
  Bus bus;
  int32_t call_count{0};

  {
    auto listener = Listener<Bus>::create_not_owning(bus);
    listener->listen([&](const Value &event) {
      ASSERT_TRUE(event.value == 3);
      ++call_count;
    });
    bus.postpone(Value{3});
    ASSERT_TRUE(bus.process() == 1);
    ASSERT_TRUE(call_count == 1);
  }

  bus.postpone(Value{2});
  ASSERT_TRUE(bus.process() == 1);
  ASSERT_TRUE(call_count == 1);
}

// TEST_CASE("Should keep listeners When listener is moved", "[Bus][Listener]")
TEST_F(EventListener, testKeepListeners){
  auto bus = std::make_shared<Bus>();
  int32_t call_count{0};

  Listener<Bus> transfer_one;
  {
    Listener<Bus> listener{bus};
    listener.listen([&](const Value &event) {
      ASSERT_TRUE(event.value == 3);
      ++call_count;
    });
    bus->postpone(Value{3});
    ASSERT_TRUE(bus->process() == 1);
    ASSERT_TRUE(call_count == 1);

    transfer_one.transfer(std::move(listener));
  }

  bus->postpone(Value{3});
  ASSERT_TRUE(bus->process() == 1);
  std::cout << "call_count: " << call_count << std::endl;
  ASSERT_TRUE(call_count == 2);
}

// TEST_CASE("Should receive event When listener added AFTER event
// emit but BEFORE event porcess",
//      "[Bus][Listener]")
TEST_F(EventListener, testReceive){
  auto bus = std::make_shared<Bus>();
  int32_t call_count{0};
  bus->postpone(Value{22});

  Listener<Bus> listener{bus};
  listener.listen([&](const Value &event) {
    ASSERT_TRUE(event.value == 22);
    ++call_count;
  });

  ASSERT_TRUE(call_count == 0);
  bus->process();
  ASSERT_TRUE(call_count == 1);
}

// TEST_CASE("Should bind listen to class method 
// and unbind When clazz dtor is called",
//      "[Bus][Listener]")
TEST_F(EventListener, testUnbind) {
  auto bus = std::make_shared<Bus>();
  struct TestBind {
    int32_t call_count{0};
    Listener<Bus> listener;

    TestBind(const std::shared_ptr<Bus>& bus)
      : listener{bus} {
      listener.listen<T1>(
          std::bind(&TestBind::on_event, this, std::placeholders::_1));
    }

    void on_event(const T1 &) {
      ++call_count;
    }
  };

  bus->postpone(T1{});
  {
    TestBind bindClazz{bus};
    ASSERT_TRUE(bindClazz.call_count == 0);
    EXPECT_TRUE(bus->process() == 1);
    ASSERT_TRUE(bindClazz.call_count == 1);
  }
  bus->postpone(T1{});
  EXPECT_TRUE(bus->process() == 1);
}

static int global_call_count{0}; // bad practice but want to just test
void free_function(const T1 &) {
  ++global_call_count;
}

// TEST_CASE("Should compile When listen in different forms", "[Bus][Listener]")
TEST_F(EventListener, testDiffForms) {
  Bus bus;

  int32_t call_count{0};

  // Listen with lambda
  auto listener = Listener<Bus>::create_not_owning(bus);
  listener->listen([&](const T1 &) { ++call_count; });

  // Listen with std::function
  auto listener2 = Listener<Bus>::create_not_owning(bus);
  std::function<void(const T1&)> callback = [&](const T1&) { ++call_count; };
  listener2->listen(callback);

  // Listen with std::bind
  auto listener3 = Listener<Bus>::create_not_owning(bus);
  struct TestClazz {
    int clazz_call_count{0};
    void on_event(const T1 &) {
      ++clazz_call_count;
    }
  };
  TestClazz clazz;
  listener3->listen<T1>(
      std::bind(&TestClazz::on_event, &clazz, std::placeholders::_1));

  // Listen with free function
  auto listener4 = Listener<Bus>::create_not_owning(bus);
  listener4->listen(free_function);

  bus.postpone(T1{});
  bus.process();

  ASSERT_TRUE(global_call_count == 1);
  ASSERT_TRUE(clazz.clazz_call_count == 1);
  ASSERT_TRUE(call_count == 2);
}

// TEST_CASE("Should NOT be able to add multiple 
//    same event callbacks When using same listener",
//      "[Bus][Listener]")
TEST_F(EventListener, testMultiple) {
  // User should use separate Listener instance as it would be unabigious 
  // what should happen when call unlisten<Event>
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);

  listener->listen([](const Value &) {});
  ASSERT_ANY_THROW(listener->listen([](const Value &) {}));
}

//TEST_CASE("Should compile", "[Bus][Listener]")
TEST_F(EventListener, testCompile) {
  // Test case to check for compilation
  Bus bus;
  {
    auto listener = Listener<Bus>::create_not_owning(bus);
    const auto callback = [](const Value &) {};
    listener->listen(callback);
  }
  {
    auto listener = Listener<Bus>::create_not_owning(bus);
    auto callback = [](const Value &) {};
    listener->listen(callback);
  }
  {
    auto listener = Listener<Bus>::create_not_owning(bus);
    auto callback = [](const Value &) {};
    listener->listen(std::move(callback));
  }
  {
    auto listener = Listener<Bus>::create_not_owning(bus);
    listener->listen([](const Value &) {});
  }
}

class TestClazz {

 public:

   static int32_t counter;

   TestClazz(int32_t id, const std::shared_ptr<Bus> &bus)
     : id_{id}
     , listener_{bus} {
     register_listener();
   }

   TestClazz(TestClazz &&other)
     : id_{other.id_}
     , listener_{other.listener_.get_bus()} {
     // We need to register again
     register_listener();
   }

   ~TestClazz() {
     id_ = 0;
   }

 private:
   int32_t id_ = 0;
   Listener<Bus> listener_;

   void register_listener() {
     listener_.listen([this](const Value &) {
       if(id_ == 1) {
        ++counter;
       }
     });
   }
};

int32_t TestClazz::counter = 0;

// TEST_CASE("Should not allow for mistake with move ctor", "[Bus][Listener]")
TEST_F(EventListener, testMoveCtor) {

  /**
   * Test case TAG: FORBID_MOVElistener_
   *
   * This case is little bit complicated.
   * We can't move Bus::Listener as it capture 'this' in ctor 
   * so whenever we would use it it
   * would lead to UB.
   */
  std::shared_ptr<Bus> bus = std::make_shared<Bus>();

  std::vector<TestClazz> vector;
  vector.emplace_back(1, bus);
  vector.emplace_back(2, bus);
  vector.emplace_back(3, bus);

  bus->postpone(Value{100});
  bus->process();

  ASSERT_TRUE(TestClazz::counter == 1);
}
