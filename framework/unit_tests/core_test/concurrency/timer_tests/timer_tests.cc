#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "util/random.h"
#include "util/wait_context.h"
#include "object_observer.h"
#include "util/test_generators.h"
#include "util/throwing_executor.h"

#include <chrono>

using namespace std::chrono_literals;

namespace plain::tests {

void test_one_timer();
void test_many_timers();
void test_timer_constructor();

void test_timer_destructor_empty_timer();
void test_timer_destructor_dead_timer_queue();
void test_timer_destructor_functionality();
void test_timer_destructor_RAII();
void test_timer_destructor();

void test_timer_cancel_empty_timer();
void test_timer_cancel_dead_timer_queue();
void test_timer_cancel_before_due_time();
void test_timer_cancel_after_due_time_before_beat();
void test_timer_cancel_after_due_time_after_beat();
void test_timer_cancel_RAII();
void test_timer_cancel();

void test_timer_operator_bool();

void test_timer_set_frequency_before_due_time();
void test_timer_set_frequency_after_due_time();
void test_timer_set_frequency();

void test_timer_oneshot_timer();
void test_timer_delay_object();

void test_timer_assignment_operator_empty_to_empty();
void test_timer_assignment_operator_non_empty_to_non_empty();
void test_timer_assignment_operator_empty_to_non_empty();
void test_timer_assignment_operator_non_empty_to_empty();
void test_timer_assignment_operator_assign_to_self();
void test_timer_assignment_operator();

}  // namespace plain::tests

using namespace std::chrono;
using time_point = std::chrono::time_point<high_resolution_clock>;

namespace plain::tests {
  auto empty_callback = []() noexcept {
  };

  class counting_executor : public plain::concurrency::executor::Basic {

   private:
    std::atomic_size_t invocation_count_;

   public:
    counting_executor() :
      plain::concurrency::executor::Basic("counting executor"), invocation_count_(0) {}

    void enqueue(plain::concurrency::Task task) override {
      ++invocation_count_;
      task();
    }

    void enqueue(std::span<plain::concurrency::Task>) override {
      // do nothing
    }

    int max_concurrency_level() const noexcept override {
      return 0;
    }

    bool shutdown_requested() const noexcept override {
      return false;
    }

    void shutdown() noexcept override {
      // do nothing
    }

    size_t invocation_count() const noexcept {
      return invocation_count_.load();
    }
  };

  class recording_executor : public plain::concurrency::executor::Basic {

   public:
    mutable std::mutex lock_;
    std::vector<::time_point> time_points_;
    ::time_point start_time_;

   public:
    recording_executor() : plain::concurrency::executor::Basic("recording executor") {
      time_points_.reserve(128);
    }

    void enqueue(plain::concurrency::Task task) override {
      const auto now = high_resolution_clock::now();

      {
        std::unique_lock<std::mutex> lock(lock_);
        time_points_.emplace_back(now);
      }

      task();
    }

    void enqueue(std::span<plain::concurrency::Task>) override {
      // do nothing
    }

    int max_concurrency_level() const noexcept override {
      return 0;
    }

    bool shutdown_requested() const noexcept override {
      return false;
    }

    void shutdown() noexcept override {
      // do nothing
    }

    void start_test() {
      const auto now = high_resolution_clock::now();
      std::unique_lock<std::mutex> lock(lock_);
      start_time_ = now;
    }

    ::time_point get_start_time() {
      std::unique_lock<std::mutex> lock(lock_);
      return start_time_;
    }

    ::time_point get_first_fire_time() {
      std::unique_lock<std::mutex> lock(lock_);
      assert(!time_points_.empty());
      return time_points_[0];
    }

    std::vector<::time_point> flush_time_points() {
      std::unique_lock<std::mutex> lock(lock_);
      return std::move(time_points_);
    }

    void reset() {
      std::unique_lock<std::mutex> lock(lock_);
      time_points_.clear();
      start_time_ = high_resolution_clock::now();
    }
  };

  class timer_tester {

   private:
    const std::chrono::milliseconds due_time_;
    std::chrono::milliseconds frequency_;
    const std::shared_ptr<recording_executor>
      executor_ = std::make_shared<recording_executor>();
    const std::shared_ptr<TimerQueue> timer__queue;
    plain::Timer timer_;

    void assert_timer_stats() noexcept {
      ASSERT_EQ(timer_.get_due_time(), due_time_);
      ASSERT_EQ(timer_.get_frequency(), frequency_);
      ASSERT_EQ(timer_.get_executor().get(), executor_.get());
      ASSERT_EQ(timer_.get_timer_queue().lock(), timer__queue);
    }

    size_t calculate_due_time() noexcept {
      const auto start_time = executor_->get_start_time();
      const auto first_fire = executor_->get_first_fire_time();
      const auto due_time =
        duration_cast<milliseconds>(first_fire - start_time).count();
      return static_cast<size_t>(due_time);
    }

    std::vector<size_t> calculate_frequencies() {
      auto fire_times = executor_->flush_time_points();

      std::vector<size_t> intervals;
      intervals.reserve(fire_times.size());

      for (size_t i = 0; i < fire_times.size() - 1; i++) {
        const auto period = fire_times[i + 1] - fire_times[i];
        const auto interval = duration_cast<milliseconds>(period).count();
        intervals.emplace_back(static_cast<size_t>(interval));
      }

      return intervals;
    }

    void assert_correct_time_points() {
      const auto recorded_due_time = calculate_due_time();
      interval_ok(recorded_due_time, due_time_.count());

      const auto intervals = calculate_frequencies();
      for (auto tested_frequency : intervals) {
        interval_ok(tested_frequency, frequency_.count());
      }
    }

   public:
    timer_tester(const milliseconds due_time, const milliseconds frequency) :
      due_time_(due_time), frequency_(frequency),
      timer__queue(std::make_shared<TimerQueue>(milliseconds(60  *1000))) {}

    timer_tester(const milliseconds due_time, const milliseconds frequency,
      const std::shared_ptr<TimerQueue> &timer_queue) :
      due_time_(due_time), frequency_(frequency), timer__queue(timer_queue) {}

    void start_timer_test() {
      timer_ = timer__queue->make_timer(due_time_, frequency_, executor_, [] {
      });

      executor_->start_test();
    }

    void start_once_timer_test() {
      timer_ = timer__queue->make_one_shot_timer(due_time_, executor_, [] {
      });

      executor_->start_test();
    }

    void set_new_frequency(std::chrono::milliseconds new_frequency) noexcept {
      timer_.set_frequency(new_frequency);
      executor_->reset();
      frequency_ = new_frequency;
    }

    void test_frequencies(size_t expected_frequency) {
      const auto frequencies = calculate_frequencies();
      for (auto tested_frequency : frequencies) {
        interval_ok(tested_frequency, expected_frequency);
      }
    }

    void test() {
      assert_timer_stats();
      assert_correct_time_points();

      timer_.cancel();
    }

    void test_oneshot_timer() {
      assert_timer_stats();
      interval_ok(calculate_due_time(), due_time_.count());

      const auto frequencies = calculate_frequencies();
      ASSERT_TRUE(frequencies.empty());

      timer_.cancel();
    }

    static void interval_ok(size_t interval, size_t expected) noexcept {
      ASSERT_GE(interval, expected - 30);
      ASSERT_LE(interval, expected + 100);
    }
  };
}  // namespace plain::tests

void plain::tests::test_one_timer() {
  const auto due_time = 1250ms;
  const auto frequency = 250ms;
  const auto tq = std::make_shared<plain::TimerQueue>(milliseconds(60  *1000));

  timer_tester tester(due_time, frequency, tq);
  tester.start_timer_test();

  std::this_thread::sleep_for(20s);

  tester.test();
}

void plain::tests::test_many_timers() {
  random randomizer;
  auto timer_queue = std::make_shared<plain::TimerQueue>(milliseconds(60  *1000));
  std::list<timer_tester> testers;

  const size_t due_time_min = 100;
  const size_t due_time_max = 2'000;
  const size_t frequency_min = 100;
  const size_t frequency_max = 3'000;
  const size_t timer_count = 1'024;

  auto round_down = [](size_t num) {
    return num - num % 50;
  };

  for (size_t i = 0; i < timer_count; i++) {
    const auto due_time = round_down(randomizer(due_time_min, due_time_max));
    const auto frequency = round_down(randomizer(frequency_min, frequency_max));

    testers.emplace_front(
      milliseconds(due_time), milliseconds(frequency),
      timer_queue).start_timer_test();
  }

  std::this_thread::sleep_for(5min);

  for (auto &tester : testers) {
    tester.test();
  }
}

void plain::tests::test_timer_constructor() {
  test_one_timer();
  test_many_timers();
}

void plain::tests::test_timer_destructor_empty_timer() {
  Timer timer;
}

void plain::tests::test_timer_destructor_dead_timer_queue() {
  Timer timer;
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();

  {
    auto timer_queue = 
      std::make_shared<plain::TimerQueue>(milliseconds(60  *1000));
    timer = timer_queue->make_timer(
      10  *1000ms, 10  *1000ms, executor, empty_callback);
  }

  // nothing strange should happen here
}

void plain::tests::test_timer_destructor_functionality() {
  auto executor = std::make_shared<counting_executor>();
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);

  size_t invocation_count_before = 0;

  {
    auto timer = timer_queue->make_timer(150ms, 150ms, executor, [] {
    });

    std::this_thread::sleep_for(1500ms + 65ms);  // racy test, but supposed to generally work

    invocation_count_before = executor->invocation_count();
  }

  std::this_thread::sleep_for(2s);

  const auto invocation_count_after = executor->invocation_count();
  ASSERT_EQ(invocation_count_before, invocation_count_after);
}

void plain::tests::test_timer_destructor_RAII() {
  const size_t timer_count = 1'024;
  object_observer observer;
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto inline_executor = std::make_shared<plain::concurrency::executor::Inline>();

  std::vector<plain::Timer> timers;
  timers.reserve(timer_count);

  for (size_t i = 0; i < timer_count; i++) {
    timers.emplace_back(timer_queue->make_timer(
      1s, 1s, inline_executor, observer.get_testing_stub()));
  }

  timers.clear();

  ASSERT_TRUE(observer.wait_destruction_count(timer_count, minutes(2)));
}

void plain::tests::test_timer_destructor() {
  test_timer_destructor_empty_timer();
  test_timer_destructor_dead_timer_queue();
  test_timer_destructor_functionality();
  test_timer_destructor_RAII();
}

void plain::tests::test_timer_cancel_empty_timer() {
  Timer timer;
  timer.cancel();
}

void plain::tests::test_timer_cancel_dead_timer_queue() {
  Timer timer;

  {
    auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
    auto executor = std::make_shared<plain::concurrency::executor::Inline>();

    timer = timer_queue->make_timer(
      10  *1000ms, 10  *1000ms, executor, empty_callback);
  }

  timer.cancel();
  ASSERT_FALSE(static_cast<bool>(timer));
}

void plain::tests::test_timer_cancel_before_due_time() {
  object_observer observer;
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto ex = std::make_shared<plain::concurrency::executor::Inline>();

  auto timer = timer_queue->make_timer(1s, 1s, ex, observer.get_testing_stub());
  timer.cancel();

  ASSERT_FALSE(static_cast<bool>(timer));

  std::this_thread::sleep_for(2s);

  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(0));
  ASSERT_TRUE(observer.wait_destruction_count(1, 10s));
}

void plain::tests::test_timer_cancel_after_due_time_before_beat() {
  object_observer observer;
  wait_context wc;

  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto ex = std::make_shared<plain::concurrency::executor::Inline>();

  auto timer = timer_queue->make_timer(
    100ms, 200ms, ex, [&wc, stub = observer.get_testing_stub()]() mutable {
    stub();
    wc.notify();
  });

  // will be released after the first beat.
  wc.wait();
  timer.cancel();

  std::this_thread::sleep_for(2s);

  ASSERT_FALSE(static_cast<bool>(timer));
  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(1));
  ASSERT_TRUE(observer.wait_destruction_count(1, 10s));
}

void plain::tests::test_timer_cancel_after_due_time_after_beat() {
  object_observer observer;
  wait_context wc;

  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto ex = std::make_shared<plain::concurrency::executor::Inline>();
  constexpr size_t max_invocation_count = 4;
  std::atomic_size_t invocation_counter = 0;

  auto timer = timer_queue->make_timer(
    100ms, 200ms, ex,
    [&invocation_counter, &wc, stub = observer.get_testing_stub()]() mutable {
    stub();

    const auto c = invocation_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    if (c == max_invocation_count) {
      wc.notify();
    }
  });

  // will be released after the first beat.
  wc.wait();
  timer.cancel();

  std::this_thread::sleep_for(2s);

  ASSERT_FALSE(static_cast<bool>(timer));
  ASSERT_EQ(observer.get_execution_count(), static_cast<size_t>(4));
  ASSERT_TRUE(observer.wait_destruction_count(1, 10s));  // one instance was invoked 4 times.
}

void plain::tests::test_timer_cancel_RAII() {
  const size_t timer_count = 1'024;
  object_observer observer;
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto ex = std::make_shared<plain::concurrency::executor::Inline>();

  std::vector<plain::Timer> timers;
  timers.reserve(timer_count);

  for (size_t i = 0; i < timer_count; i++) {
    timers.emplace_back(timer_queue->make_timer(2s, 2s, ex, observer.get_testing_stub()));
  }

  for (auto &timer : timers) {
    timer.cancel();
  }

  ASSERT_TRUE(observer.wait_destruction_count(timer_count, 1min));
}

void plain::tests::test_timer_cancel() {
  test_timer_cancel_empty_timer();
  test_timer_cancel_dead_timer_queue();
  test_timer_cancel_before_due_time();
  test_timer_cancel_after_due_time_before_beat();
  test_timer_cancel_after_due_time_after_beat();
  test_timer_cancel_RAII();
}

void plain::tests::test_timer_operator_bool() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto inline_executor = std::make_shared<plain::concurrency::executor::Inline>();

  auto timer_1 = timer_queue->make_timer(
    10  *1000ms, 10  *1000ms, inline_executor, empty_callback);

  ASSERT_TRUE(static_cast<bool>(timer_1));

  auto timer_2 = std::move(timer_1);
  ASSERT_FALSE(static_cast<bool>(timer_1));
  ASSERT_TRUE(static_cast<bool>(timer_2));

  timer_2.cancel();
  ASSERT_FALSE(static_cast<bool>(timer_2));
}

void plain::tests::test_timer_set_frequency_before_due_time() {
  const auto new_frequency = 100ms;
  timer_tester tester(250ms, 250ms);
  tester.start_timer_test();
  tester.set_new_frequency(new_frequency);

  std::this_thread::sleep_for(std::chrono::seconds(2));

  tester.test_frequencies(new_frequency.count());
}

void plain::tests::test_timer_set_frequency_after_due_time() {
  const auto new_frequency = 100ms;
  timer_tester tester(150ms, 350ms);
  tester.start_timer_test();

  std::this_thread::sleep_for(215ms);

  tester.set_new_frequency(new_frequency);

  std::this_thread::sleep_for(4s);

  tester.test_frequencies(new_frequency.count());
}

void plain::tests::test_timer_set_frequency() {
  // empty timer throws
  assert_throws<std::runtime_error>([] {
    Timer timer;
    timer.set_frequency(200ms);
  });

  test_timer_set_frequency_before_due_time();
  test_timer_set_frequency_after_due_time();
}

void plain::tests::test_timer_oneshot_timer() {
  timer_tester tester(150ms, 0ms);
  tester.start_once_timer_test();

  std::this_thread::sleep_for(3s);

  tester.test_oneshot_timer();
}

void plain::tests::test_timer_delay_object() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto wt_executor = 
    std::make_shared<plain::concurrency::executor::WorkerThread>();
  const auto expected_interval = 150ms;

  for (size_t i = 0; i < 15; i++) {
    const auto before = high_resolution_clock::now();
    auto delay_object = 
      timer_queue->make_delay_object(expected_interval, wt_executor);
    ASSERT_TRUE(static_cast<bool>(delay_object));
    delay_object.run().get();
    const auto after = high_resolution_clock::now();
    const auto interval_ms = duration_cast<milliseconds>(after - before);
    timer_tester::interval_ok(interval_ms.count(), expected_interval.count());
  }

  wt_executor->shutdown();
}

void plain::tests::test_timer_assignment_operator_empty_to_empty() {
  plain::Timer timer1, timer2;
  ASSERT_FALSE(static_cast<bool>(timer1));
  ASSERT_FALSE(static_cast<bool>(timer2));

  timer1 = std::move(timer2);

  ASSERT_FALSE(static_cast<bool>(timer1));
  ASSERT_FALSE(static_cast<bool>(timer2));
}

void plain::tests::test_timer_assignment_operator_non_empty_to_non_empty() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto executor0 = std::make_shared<plain::concurrency::executor::Inline>();
  auto executor1 = std::make_shared<plain::concurrency::executor::Inline>();

  object_observer observer0, observer1;

  auto timer0 = timer_queue->make_timer(
    10s, 10s, executor0, observer0.get_testing_stub());
  auto timer1 = timer_queue->make_timer(
    20s, 20s, executor1, observer1.get_testing_stub());

  timer0 = std::move(timer1);

  ASSERT_TRUE(static_cast<bool>(timer0));
  ASSERT_FALSE(static_cast<bool>(timer1));

  ASSERT_TRUE(observer0.wait_destruction_count(1, 20s));
  ASSERT_EQ(observer1.get_destruction_count(), static_cast<size_t>(0));

  ASSERT_EQ(timer0.get_due_time(), 20s);
  ASSERT_EQ(timer0.get_frequency(), 20s);
  ASSERT_EQ(timer0.get_executor(), executor1);
}

void plain::tests::test_timer_assignment_operator_empty_to_non_empty() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  object_observer observer;

  auto timer0 = timer_queue->make_timer(
    10s, 10s, executor, observer.get_testing_stub());
  plain::Timer timer1;

  timer0 = std::move(timer1);
  ASSERT_FALSE(static_cast<bool>(timer0));
  ASSERT_FALSE(static_cast<bool>(timer1));

  ASSERT_TRUE(observer.wait_destruction_count(1, 20s));
}

void plain::tests::test_timer_assignment_operator_non_empty_to_empty() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  object_observer observer;

  plain::Timer timer0;
  auto timer1 = timer_queue->make_timer(
    10s, 10s, executor, observer.get_testing_stub());

  timer0 = std::move(timer1);

  ASSERT_FALSE(static_cast<bool>(timer1));
  ASSERT_TRUE(static_cast<bool>(timer0));

  ASSERT_EQ(timer0.get_due_time(), 10s);
  ASSERT_EQ(timer0.get_frequency(), 10s);
  ASSERT_EQ(timer0.get_executor(), executor);
}

void plain::tests::test_timer_assignment_operator_assign_to_self() {
  auto timer_queue = std::make_shared<plain::TimerQueue>(120s);
  auto executor = std::make_shared<plain::concurrency::executor::Inline>();
  object_observer observer;

  auto timer = timer_queue->make_timer(1s, 1s, executor, observer.get_testing_stub());

  timer = std::move(timer);

  ASSERT_TRUE(static_cast<bool>(timer));
  ASSERT_EQ(timer.get_due_time(), 1s);
  ASSERT_EQ(timer.get_frequency(), 1s);
  ASSERT_EQ(timer.get_executor(), executor);
}

void plain::tests::test_timer_assignment_operator() {
  test_timer_assignment_operator_empty_to_empty();
  test_timer_assignment_operator_non_empty_to_non_empty();
  test_timer_assignment_operator_empty_to_non_empty();
  test_timer_assignment_operator_non_empty_to_empty();
  test_timer_assignment_operator_assign_to_self();
}

using namespace plain::tests;

class Timer : public testing::Test {

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

/*
TEST_F(Timer, testConstructor) {
  test_timer_constructor();
}

TEST_F(Timer, testDestructor) {
  test_timer_destructor();
}

TEST_F(Timer, testCancel) {
  test_timer_cancel();
}

TEST_F(Timer, testOperatorBool) {
  test_timer_operator_bool();
}

TEST_F(Timer, testSetFrequency) {
  test_timer_set_frequency();
}

TEST_F(Timer, testOneshotTimer) {
  test_timer_oneshot_timer();
}

TEST_F(Timer, testDelayObject) {
  test_timer_delay_object();
}

TEST_F(Timer, testOperatorAssignment) {
  test_timer_assignment_operator();
}
*/

/*
int main() {
  tester test("timer test");

  test.add_step("constructor", test_timer_constructor);
  test.add_step("destructor", test_timer_destructor);
  test.add_step("cancel", test_timer_cancel);
  test.add_step("operator bool", test_timer_operator_bool);
  test.add_step("set_frequency", test_timer_set_frequency);
  test.add_step("oneshot_timer", test_timer_oneshot_timer);
  test.add_step("delay_object", test_timer_delay_object);
  test.add_step("operator =", test_timer_assignment_operator);

  test.launch_test();
  return 0;
}
*/
