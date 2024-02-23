#include "plain/all.h"

#include "gtest/gtest.h"
#include "assertions.h"
#include "object_observer.h"
#include "util/test_ready_result.h"

namespace plain::tests {

template<class type>
void test_make_ready_result_impl();
void test_make_ready_result();

template<class type>
void test_make_exceptional_result_impl();
void test_make_exceptional_result();

}  // namespace plain::tests

template<class type>
void plain::tests::test_make_ready_result_impl() {
  plain::concurrency::Result<type> result;

  if constexpr (std::is_same_v<void, type>) {
    result = plain::concurrency::result::make_ready<type>();
  } else {
    result = plain::concurrency::result::make_ready<type>(
      value_gen<type>::default_value());
  }

  test_ready_result(std::move(result));
}

void plain::tests::test_make_ready_result() {
  test_make_ready_result_impl<int>();
  test_make_ready_result_impl<std::string>();
  test_make_ready_result_impl<void>();
  test_make_ready_result_impl<int&>();
  test_make_ready_result_impl<std::string&>();
}

template<class type>
void plain::tests::test_make_exceptional_result_impl() {
  // empty exception_ptr makes make_exceptional_result throw.
  assert_throws_with_error_message<std::invalid_argument>(
    [] {
      plain::concurrency::result::make_exceptional<type>({});
    },
    "make_exceptional exception null");

  const size_t id = 123456789;
  auto res0 = plain::concurrency::result::make_exceptional<type>(custom_exception(id));
  test_ready_result_custom_exception(std::move(res0), id);

  auto res1 = plain::concurrency::result::make_exceptional<type>(
    std::make_exception_ptr(custom_exception(id)));
  test_ready_result_custom_exception(std::move(res1), id);
}

void plain::tests::test_make_exceptional_result() {
  test_make_exceptional_result_impl<int>();
  test_make_exceptional_result_impl<std::string>();
  test_make_exceptional_result_impl<void>();
  test_make_exceptional_result_impl<int&>();
  test_make_exceptional_result_impl<std::string&>();
}

using namespace plain::tests;

class MakeResult : public testing::Test {

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

TEST_F(MakeResult, testReady) {
  test_make_ready_result();
}

TEST_F(MakeResult, testExceptional) {
  test_make_exceptional_result();
}

/*
int main() {
  tester tester("make_result test");

  tester.add_step("make_ready_result", test_make_ready_result);
  tester.add_step("make_exceptional_result", test_make_exceptional_result);

  tester.launch_test();
  return 0;
}
*/
