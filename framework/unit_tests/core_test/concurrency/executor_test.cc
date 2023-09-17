#include "gtest/gtest.h"
#include <algorithm>
#include "plain/all.h"

using namespace plain::concurrency;

class Executor : public testing::Test {

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

std::vector<int> make_random_vector() {
  std::vector<int> vec(64 * 1'024);
  std::srand(std::time(nullptr));
  for (auto &i : vec) {
    i = ::rand();
  }
  return vec;
}

Result<size_t>
count_even(
  std::shared_ptr<executor::ThreadPool> tpe, const std::vector<int> &vector) {

  const auto vecor_size = vector.size();
  const auto concurrency_level = tpe->max_concurrency_level();
  const auto chunk_size = vecor_size / concurrency_level;

  std::vector<Result<size_t>> chunk_count;

  for (auto i = 0; i < concurrency_level; i++) {
    const auto chunk_begin = i * chunk_size;
    const auto chunk_end = chunk_begin + chunk_size;
    auto result = tpe->submit([&vector, chunk_begin, chunk_end]() -> size_t {
      return std::count_if(
        vector.begin() + chunk_begin, vector.begin() + chunk_end, [](auto i) {
        return i % 2 == 0;
      });
    });

    chunk_count.emplace_back(std::move(result));
  }

  size_t total_count = 0;

  for (auto& result : chunk_count) {
    total_count += co_await result;
  }

  co_return total_count;
}

TEST_F(Executor, testHello) {
  plain::Kernel kernel;
  auto result = kernel.thread_executor()->submit([] {
    std::cout << "Hello world" << std::endl;
  });
  result.get();
}

TEST_F(Executor, testCountEven) {
  plain::Kernel kernel;
  const auto vector = make_random_vector();
  auto result = count_even(kernel.thread_pool_executor(), vector);
  const auto total_count = result.get();
  std::cout << 
    "There are " << total_count << " even numbers in the vector" << std::endl;
}
