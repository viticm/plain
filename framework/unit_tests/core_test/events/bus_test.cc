#include "event.h"
#include "gtest/gtest.h"
#include "pf/events/bus.h"

using namespace pf_events;

class EventBus : public testing::Test {

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

TEST_F(EventBus, testDeliver) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus); 
  int32_t call_count{0};
  listener->listen([&](const Value &event) {
    ASSERT_TRUE(3 == event.value);
    ++call_count;
  });
  ASSERT_TRUE(0 == call_count);
  bus.postpone(Value{3});
  ASSERT_TRUE(0 == call_count);
  ASSERT_TRUE(1 == bus.process());
  ASSERT_TRUE(1 == call_count);
}

TEST_F(EventBus, testReplace) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  {
    listener->listen([&](const Value &event) {
      ASSERT_TRUE(3 == event.value);
      ++call_count;
    });
    bus.postpone(Value{3});
    ASSERT_TRUE(1 == bus.process());
    ASSERT_TRUE(1 == call_count);
  }
  {
    listener->unlisten<Value>();
    bus.postpone(Value{2});
    ASSERT_TRUE(1 == bus.process());
    ASSERT_TRUE(1 == call_count);
  }
  {
    listener->listen([&](const Value &event) { 
      ASSERT_TRUE(1 == event.value);
      ++call_count;
    });
    bus.postpone(Value{1});
    ASSERT_TRUE(1 == bus.process());
    ASSERT_TRUE(2 == call_count);
  }
}

TEST_F(EventBus, testReplace1) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  {
    listener->listen([&](const Value &event) {
      ASSERT_TRUE(3 == event.value);
      ++call_count;
    });
    listener->listen([&](const T1 &) { ++call_count; });
    bus.postpone(Value{3});
    bus.postpone(T1{});
    ASSERT_TRUE(2 == bus.process());
    ASSERT_TRUE(2 == call_count);
  }

  {
    listener->unlisten_all();
    bus.postpone(Value{3});
    bus.postpone(T1{});
    ASSERT_TRUE(2 == bus.process());
    ASSERT_TRUE(2 == call_count);
  }

  {
    listener->listen([&](const Value &event) {
      ASSERT_TRUE(1 == event.value);
      ++call_count;
    });
    bus.postpone(Value{1});
    bus.postpone(T1{});
    ASSERT_TRUE(2 == bus.process());
    ASSERT_TRUE(3 == call_count);
  }
}

TEST_F(EventBus, testFlush) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  listener->listen([&](const Value &) {
    ++call_count;
    listener->unlisten<Value>();
  });
  bus.postpone(Value{3});
  bus.postpone(Value{3});
  bus.postpone(Value{3});
  ASSERT_TRUE(3 == bus.process());
  ASSERT_TRUE(1 == call_count);
}

TEST_F (EventBus, testMultiple) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  listener->listen([&](const Value &) {
    ASSERT_NO_THROW(listener->unlisten<Value>());
    ASSERT_NO_THROW(listener->unlisten<Value>());
    ASSERT_NO_THROW(listener->unlisten<Value>());
    ++call_count;
  });
  bus.postpone(Value{3});
  bus.postpone(Value{3});
  bus.postpone(Value{3});
  ASSERT_TRUE(3 == bus.process());
  ASSERT_TRUE(1 == call_count);
}

TEST_F(EventBus, testUnique) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  ASSERT_NO_THROW(listener->listen([](const T1&) {}));
  ASSERT_ANY_THROW(listener->listen([](const T1&) {}));
}

TEST_F(EventBus, testProcessing) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  auto listener_other = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  int32_t call_count_other{0};
  listener->listen([&](const Value &) {
    ++call_count;
    if (1 == call_count) { // Remember that we can only add it once! 
      listener_other->listen([&call_count_other](const Value &) { 
        ++call_count_other;  
      });
    }
  });

  {
    bus.postpone(Value{3});
    ASSERT_TRUE(1 == bus.process());
    ASSERT_TRUE(1 == call_count);
    ASSERT_TRUE(0 == call_count_other);

    bus.postpone(Value{3});
    ASSERT_TRUE(1 == bus.process());
    ASSERT_TRUE(2 == call_count);
    ASSERT_TRUE(1 == call_count_other);
  }

  {
    listener_other->unlisten_all();
    call_count = 0;
    call_count_other = 0;
    bus.postpone(Value{3});
    bus.postpone(Value{3});
    bus.postpone(Value{3});
    ASSERT_TRUE(3 == bus.process()); // It shouldn't accumulate events 
    ASSERT_TRUE(3 == call_count);
    ASSERT_TRUE(2 == call_count_other);
  }

}

TEST_F(EventBus, testUnlistenProcessing) {

  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  auto listener_other = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  int32_t call_count_other{0};
  listener->listen([&](const Value &) {
    ++call_count;
    if (1 == call_count) { // Remember that we can only add it once! 
      listener_other->listen([&call_count_other](const Value &) { 
          ++call_count_other;  
      });
    }
    listener->unlisten_all();
  });

  bus.postpone(Value{3}); 
  bus.postpone(Value{3}); 
  bus.postpone(Value{3}); 

  ASSERT_TRUE(3 == bus.process());
  ASSERT_TRUE(1 == call_count);
  ASSERT_TRUE(2 == call_count_other);

}

TEST_F(EventBus, testMatryoshkaStyle) {

  Bus bus;
  auto listener1 = Listener<Bus>::create_not_owning(bus);
  auto listener2 = Listener<Bus>::create_not_owning(bus);
  auto listener3 = Listener<Bus>::create_not_owning(bus);

  int32_t call_count{0};

  listener1->listen([&](const Value &) { 
    listener1->unlisten_all(); // Level 1.
    
    listener2->listen([&](const Value &) { 
      listener2->unlisten_all(); // Level 2.
      
      listener3->listen([&](const Value &) { 
        listener3->unlisten_all(); // Level 3.
        ++call_count;
      });

      ++call_count;
    });

    ++call_count;
  });
  
  bus.postpone(Value{3});
  bus.postpone(Value{3});
  bus.postpone(Value{3});
  bus.postpone(Value{3});

  ASSERT_TRUE(4 == bus.process());
  ASSERT_TRUE(3 == call_count);
}

TEST_F(EventBus, testChain) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);
  auto listener_other = Listener<Bus>::create_not_owning(bus);
  int32_t call_count{0};
  int32_t call_other_option{0};
 
  listener->listen([&](const Value &) {
    ++call_count;
    // Remember that we can only add it once!
    listener_other->listen([&](const Value &) { call_other_option = 1; });
    listener_other->unlisten<Value>();
    listener_other->listen([&](const Value &) { call_other_option = 2; });
    listener_other->unlisten<Value>();
    listener_other->listen([&](const Value &) { call_other_option = 3; });
    listener_other->unlisten<Value>();
    
    listener_other->listen([&](const Value &) { call_other_option = 4; });
    
    listener->unlisten_all();
  });

  bus.postpone(Value{3});
  bus.postpone(Value{3});
  bus.postpone(Value{3});

  ASSERT_TRUE(3 == bus.process());
  ASSERT_TRUE(1 == call_count);
  ASSERT_TRUE(4 == call_other_option);
}

TEST_F(EventBus, testNotProcess) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);

  bus.postpone(Value{3});
  bus.postpone(Value{3});
  bus.postpone(Value{3});

  ASSERT_TRUE(3 == bus.process());
  ASSERT_TRUE(0 == bus.process());
}

TEST_F(EventBus, testProcessTransit) {
  Bus bus;
 
  // This case may be usefull when we use EventBus for some kind of state 
  // machine and we are during transit from one state to other.

  auto listener_a = Listener<Bus>::create_not_owning(bus);
  auto listener_b = Listener<Bus>::create_not_owning(bus);

  int32_t listener_a_receive_event{0};
  int32_t listener_b_receive_event{0};

  listener_a->listen([&](const Value &) {
    ++listener_b_receive_event;
  });

  ASSERT_TRUE(0 == bus.process());

  {
    bus.postpone(Value{3}); // <-- before
    listener_a->unlisten_all();
    listener_b->listen([&](const Value &) { ++listener_b_receive_event; }); 
  }

  /*
  {
    listener_a->unlisten_all();
    bus.postpone(Value{3}); // <-- in
    listener_b->listen([&](const Value &) { ++listener_b_receive_event; }); 
  }
 
  {
    listener_a->unlisten_all();
    listener_b->listen([&](const Value &) { ++listener_b_receive_event; }); 
    bus.postpone(Value{3}); // <-- in
  }
  */

  ASSERT_TRUE(1 == bus.process());
  ASSERT_TRUE(0 == listener_a_receive_event);
  ASSERT_TRUE(1 == listener_b_receive_event);
}

TEST_F(EventBus, testProcessNot) {
  Bus bus;
  auto listener = Listener<Bus>::create_not_owning(bus);

  int32_t listener_receive_event{0};

  listener->listen([&](const Value &) { ++listener_receive_event; });

  ASSERT_TRUE(0 == bus.process());
  bus.postpone(Value{3});
  ASSERT_TRUE(1 == bus.process());
  EXPECT_TRUE(1 == listener_receive_event);

  // All cases should be same because of deterministic way of processing.
  {
    bus.postpone(Value{3}); // <-- before 
    listener->unlisten_all();
  }

  /*
  {
    listener->unlisten_all();
    bus.postpone(Value{3}); // <-- after
  }
  */
  
  ASSERT_TRUE(1 == bus.process());
  EXPECT_TRUE(1 == listener_receive_event);
}
