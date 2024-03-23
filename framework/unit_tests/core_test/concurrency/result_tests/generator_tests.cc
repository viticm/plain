#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_generators.h"

using namespace plain::tests;

namespace plain::tests {

void test_generator_move_constructor();
void test_generator_destructor();
void test_generator_begin();

template<class type>
void test_generator_begin_end_impl();
void test_generator_begin_end();

void test_generator_iterator_operator_plus_plus_exception();
void test_generator_iterator_operator_plus_plus_ran_to_end();
void test_generator_iterator_operator_plus_plus();

void test_generator_iterator_operator_plus_plus_postfix_exception();
void test_generator_iterator_operator_plus_plus_postfix_ran_to_end();
void test_generator_iterator_operator_plus_plus_postfix();

void test_generator_iterator_dereferencing_operators();
void test_generator_iterator_comparison_operators();

}  // namespace plain::tests

void plain::tests::test_generator_move_constructor() {
  auto gen0 = []() -> plain::concurrency::result::Generator<int> {
    co_yield 1;
  }();

  ASSERT_TRUE(static_cast<bool>(gen0));

  plain::concurrency::result::Generator<int> gen1(std::move(gen0));
  ASSERT_FALSE(static_cast<bool>(gen0));
  ASSERT_TRUE(static_cast<bool>(gen1));
}

void plain::tests::test_generator_destructor() {
  auto gen_fn = [](testing_stub stub) -> plain::concurrency::result::Generator<int> {
    co_yield 1;
  };

  object_observer observer;

  {
    auto gen0 = gen_fn(observer.get_testing_stub());
    auto gen1(std::move(gen0));  // check to see that empty generator d.tor is benign
  }

  ASSERT_EQ(observer.get_destruction_count(), 1);
}

void plain::tests::test_generator_begin() {
  auto gen0 = []() -> plain::concurrency::result::Generator<int> {
    throw custom_exception(1234567);
    co_yield 1;
  }();

  assert_throws<custom_exception>([&gen0] {
    gen0.begin();
  });

  auto gen1 = []() -> plain::concurrency::result::Generator<int> {
    co_yield 1;
  }();

  auto gen2(std::move(gen1));
  assert_throws_with_error_message<std::runtime_error>(
    [&gen1] {
      gen1.begin();
    },
    "begin - generator is empty.");
}

template<class type>
void plain::tests::test_generator_begin_end_impl() {
  value_gen<type> val_gen;
  auto gen = [&val_gen]() -> plain::concurrency::result::Generator<type> {
    for (size_t i = 0; i < 1024; i++) {
      co_yield val_gen.value_of(i);
    }
  };

  size_t counter = 0;
  for (const auto &val : gen()) {
    if constexpr (std::is_reference_v<type>) {
      ASSERT_EQ(&val, &val_gen.value_of(counter));
    } else {
      ASSERT_EQ(val, val_gen.value_of(counter));
    }
    counter++;
  }

  ASSERT_EQ(counter, 1024);
}

void plain::tests::test_generator_begin_end() {
  test_generator_begin_end_impl<int>();
  test_generator_begin_end_impl<std::string>();
  test_generator_begin_end_impl<int&>();
  test_generator_begin_end_impl<std::string&>();
}

void plain::tests::test_generator_iterator_operator_plus_plus_exception() {
  auto gen = []() -> plain::concurrency::result::Generator<int> {
    for (auto i = 0; i < 10; i++) {
      if (i != 0  &&i % 3 == 0) {
        throw custom_exception(i);
      }

      co_yield i;
    }
  }();

  auto gen_it = gen.begin();  // i = 0
  const auto end = gen.end();
  ASSERT_NE(gen_it, end);

  auto &res0 = ++gen_it;  // i = 1
  ASSERT_EQ(&res0, &gen_it);
  ASSERT_NE(gen_it, end);

  auto &res1 = ++gen_it;  // i = 2
  ASSERT_EQ(&res1, &gen_it);
  ASSERT_NE(gen_it, end);

  assert_throws<custom_exception>([&gen_it] {
    ++gen_it;
  });  // i = 3
  ASSERT_EQ(gen_it, end);
}

void plain::tests::test_generator_iterator_operator_plus_plus_ran_to_end() {
  auto gen = []() -> plain::concurrency::result::Generator<int> {
    for (auto i = 0; i < 3; i++) {
      co_yield i;
    }
  }();

  auto gen_it = gen.begin();  // i = 0
  const auto end = gen.end();
  ASSERT_NE(gen_it, end);

  auto &res0 = ++gen_it;  // i = 1
  ASSERT_EQ(&res0, &gen_it);
  ASSERT_NE(gen_it, end);

  auto &res1 = ++gen_it;  // i = 2
  ASSERT_EQ(&res1, &gen_it);
  ASSERT_NE(gen_it, end);

  auto &res2 = ++gen_it;  // i = 3
  ASSERT_EQ(&res2, &gen_it);
  ASSERT_EQ(gen_it, end);
}

void plain::tests::test_generator_iterator_operator_plus_plus() {
  test_generator_iterator_operator_plus_plus_exception();
  test_generator_iterator_operator_plus_plus_ran_to_end();
}

void
plain::tests::test_generator_iterator_operator_plus_plus_postfix_exception() {
  auto gen = []() -> plain::concurrency::result::Generator<int> {
    for (auto i = 0; i < 10; i++) {
      if (i != 0  &&i % 3 == 0) {
        throw custom_exception(i);
      }

      co_yield i;
    }
  }();

  auto gen_it = gen.begin();  // i = 0
  const auto end = gen.end();
  ASSERT_NE(gen_it, end);

  gen_it++;  // i = 1
  ASSERT_NE(gen_it, end);

  gen_it++;  // i = 2
  ASSERT_NE(gen_it, end);

  assert_throws<custom_exception>([&gen_it] {
    gen_it++;
  });  // i = 3

  ASSERT_EQ(gen_it, end);
}

void
plain::tests::test_generator_iterator_operator_plus_plus_postfix_ran_to_end() {
  auto gen = []() -> plain::concurrency::result::Generator<int> {
    for (auto i = 0; i < 3; i++) {
      co_yield i;
    }
  }();

  auto gen_it = gen.begin();  // i = 0
  const auto end = gen.end();
  ASSERT_NE(gen_it, end);

  gen_it++;  // i = 1
  ASSERT_NE(gen_it, end);

  gen_it++;  // i = 2
  ASSERT_NE(gen_it, end);

  gen_it++;  // i = 3
  ASSERT_EQ(gen_it, end);
}

void plain::tests::test_generator_iterator_operator_plus_plus_postfix() {
  test_generator_iterator_operator_plus_plus_postfix_exception();
  test_generator_iterator_operator_plus_plus_postfix_ran_to_end();
}

void plain::tests::test_generator_iterator_dereferencing_operators() {
  struct dummy {
    int i = 0;
  };

  dummy arr[6] = {};
  auto gen = [](std::span<dummy> s) -> plain::concurrency::result::Generator<dummy> {
    int i = 0;
    while (true) {
      co_yield s[i % s.size()];
      ++i;
    }
  }(arr);

  auto it = gen.begin();
  for (size_t i = 0; i < 100; i++) {
    auto &ref = *it;
    auto *ptr = &it->i;

    auto &expected = arr[i % std::size(arr)];
    ASSERT_EQ(&ref, &expected);
    ASSERT_EQ(ptr, &expected.i);
    ++it;
  }
}

void plain::tests::test_generator_iterator_comparison_operators() {
  auto gen = []() -> plain::concurrency::result::Generator<int> {
    co_yield 1;
    co_yield 2;
  };

  auto gen0 = gen();
  auto gen1 = gen();

  const auto it0 = gen0.begin();
  const auto it1 = gen1.begin();
  const auto copy = it0;
  const auto end = gen0.end();

  // operator ==
  {
    ASSERT_TRUE(it0 == it0);
    ASSERT_TRUE(it0 == copy);
    ASSERT_TRUE(copy == it0);
    ASSERT_FALSE(it0 == it1);
    ASSERT_FALSE(it1 == it0);
    ASSERT_FALSE(it0 == end);
    ASSERT_FALSE(end == it0);
  }

  // operator !=
  {
    ASSERT_FALSE(it0 != it0);
    ASSERT_FALSE(it0 != copy);
    ASSERT_FALSE(copy != it0);
    ASSERT_TRUE(it0 != it1);
    ASSERT_TRUE(it1 != it0);
    ASSERT_TRUE(it0 != end);
    ASSERT_TRUE(end != it0);
  }
}

class ResultGenerator : public testing::Test {

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

TEST_F(ResultGenerator, testMoveConstructor) {
  test_generator_move_constructor();
}

TEST_F(ResultGenerator, testDestructor) {
  test_generator_destructor();
}

TEST_F(ResultGenerator, testBegin) {
  test_generator_begin();
}

TEST_F(ResultGenerator, testBeginAndEnd) {
  test_generator_begin_end();
}

TEST_F(ResultGenerator, testOperatorPlusPlusIt) {
  test_generator_iterator_operator_plus_plus();
}

TEST_F(ResultGenerator, testOperatorItPlusPlus) {
  test_generator_iterator_operator_plus_plus_postfix();
}

TEST_F(ResultGenerator, testOperatorItDereferencing) {
  test_generator_iterator_dereferencing_operators();
}

TEST_F(ResultGenerator, testOperatorItComparison) {
  test_generator_iterator_comparison_operators();
}

/*
int main() {
  {
    tester tester("generator test");

    tester.add_step("move constructor", test_generator_move_constructor);
    tester.add_step("destructor", test_generator_destructor);
    tester.add_step("begin", test_generator_begin);
    tester.add_step("begin + end", test_generator_begin_end);

    tester.launch_test();
  }

  {
    tester tester("generator_iterator test");

    tester.add_step("operator ++it", test_generator_iterator_operator_plus_plus);
    tester.add_step("operator it++", test_generator_iterator_operator_plus_plus_postfix);
    tester.add_step("operator *, operator ->", test_generator_iterator_dereferencing_operators);
    tester.add_step("operator ==, operator !=", test_generator_iterator_comparison_operators);

    tester.launch_test();
  }

  return 0;
}
*/
