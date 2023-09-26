#ifndef PLAIN_CORE_TEST_RANDOM_H_
#define PLAIN_CORE_TEST_RANDOM_H_

#include <random>

namespace plain::tests {
  class random {

   private:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<intptr_t> dist;

   public:
    random() noexcept : mt(rd()), dist(-1'000'000, 1'000'000) {}

    intptr_t operator()() {
      return dist(mt);
    }

    int64_t operator()(int64_t min, int64_t max) {
      const auto r = (*this)();
      const auto upper_limit = max - min + 1;
      return std::abs(r) % upper_limit + min;
    }
  };
}  // namespace plain::tests

#endif  // PLAIN_CORE_TEST_RANDOM_H_
