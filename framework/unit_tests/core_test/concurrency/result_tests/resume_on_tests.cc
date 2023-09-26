#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"

#include <unordered_set>

namespace plain::tests {

void test_resume_on_null_executor();
void test_resume_on_shutdown_executor();
void test_resume_on_shutdown_executor_delayed();
void test_resume_on_shared_ptr();
void test_resume_on_ref();

}  // namespace plain::tests

namespace plain::tests {

concurrency::Result<void>
resume_on_1_executor(
  std::shared_ptr<plain::concurrency::executor::Basic> executor) {
  co_await plain::concurrency::result::resume_on(executor);
}

concurrency::Result<void> 
resume_on_many_executors_shared(
  std::span<std::shared_ptr<plain::concurrency::executor::Basic>> executors,
  std::unordered_set<size_t> &set) {
  for (auto &executor : executors) {
    co_await plain::concurrency::result::resume_on(executor);
    set.insert(::plain::thread::get_current_virtual_id());
  }
}

concurrency::Result<void> 
resume_on_many_executors_ref(
  std::span<plain::concurrency::executor::Basic*> executors,
  std::unordered_set<size_t> &set) {
  for (auto executor : executors) {
    co_await plain::concurrency::result::resume_on(*executor);
    set.insert(::plain::thread::get_current_virtual_id());
  }
}

}  // namespace plain::tests

void plain::tests::test_resume_on_null_executor() {
  assert_throws_with_error_message<std::invalid_argument>(
    [] {
      resume_on_1_executor({}).get();
    },
    "resume_on - given executor is null.");
}

void plain::tests::test_resume_on_shutdown_executor() {
  auto ex = std::make_shared<plain::concurrency::executor::Inline>();
  ex->shutdown();

  assert_throws<std::runtime_error>([ex] {
    resume_on_1_executor(ex).get();
  });
}

void plain::tests::test_resume_on_shutdown_executor_delayed() {
  auto ex = std::make_shared<plain::concurrency::executor::Manual>();
  auto result = resume_on_1_executor(ex);

  ASSERT_EQ(ex->size(), 1);

  ex->shutdown();

  assert_throws<std::runtime_error>([&result] {
    result.get();
  });
}

void plain::tests::test_resume_on_shared_ptr() {

  plain::Kernel runtime;
  std::shared_ptr<plain::concurrency::executor::Basic> executors[4];

  executors[0] = runtime.thread_executor();
  executors[1] = runtime.thread_pool_executor();
  executors[2] = runtime.make_worker_thread_executor();
  executors[3] = runtime.make_worker_thread_executor();

  std::unordered_set<size_t> set;
  resume_on_many_executors_shared(executors, set).get();

  ASSERT_EQ(set.size(), std::size(executors));
}

void plain::tests::test_resume_on_ref() {
  plain::Kernel runtime;
  plain::concurrency::executor::Basic *executors[4];

  executors[0] = runtime.thread_executor().get();
  executors[1] = runtime.thread_pool_executor().get();
  executors[2] = runtime.make_worker_thread_executor().get();
  executors[3] = runtime.make_worker_thread_executor().get();

  std::unordered_set<size_t> set;
  resume_on_many_executors_ref(executors, set).get();

  ASSERT_EQ(set.size(), std::size(executors));
}

using namespace plain::tests;

class ResumeOn : public testing::Test {

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

TEST_F(ResumeOn, testNullptr) {
  test_resume_on_null_executor();
}

TEST_F(ResumeOn, testExecutorWasShutDown) {
  test_resume_on_shutdown_executor();
}

TEST_F(ResumeOn, testExecutorIsShutDownAfterEnqueuing) {
  test_resume_on_shutdown_executor_delayed();
}

TEST_F(ResumeOn, testSharedPtr) {
  test_resume_on_shared_ptr();
}

TEST_F(ResumeOn, testRef) {
  test_resume_on_ref();
}

/*
int main() {
  tester tester("resume_on");

  tester.add_step("resume_on(nullptr)", test_resume_on_null_executor);
  tester.add_step("resume_on - executor was shut down", test_resume_on_shutdown_executor);
  tester.add_step("resume_on - executor is shut down after enqueuing", test_resume_on_shutdown_executor_delayed);
  tester.add_step("resume_on(std::shared_ptr)", test_resume_on_shared_ptr);
  tester.add_step("resume_on(&)", test_resume_on_ref);

  tester.launch_test();
  return 0;
}
*/
