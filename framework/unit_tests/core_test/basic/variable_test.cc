#include "gtest/gtest.h"
#include "plain/basic/type/variable.h"

class Variable : public testing::Test {

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

void variable_all() {
  using namespace plain;
  variable_t a = 1;
  variable_t b = "111";
  variable_t c = 20.3f;
  variable_t d = true;
  variable_t f = 0.1;
  uint32_t g = 1;
  int64_t h = 100000;
  std::cout << a << std::endl;
  ++a;
  std::cout << a << std::endl;
  a++;
  std::cout << a << std::endl;
  std::cout << 1 << std::endl;
  std::cout << g << std::endl;
  std::cout << h << std::endl;
  std::cout <<
    "a: " << a <<
    " b: " << b <<
    " c: " << c <<
    " d: " << d <<
    " f: " << f <<
    " ~g: " << variable_t{g} <<
    " ~h: " << variable_t{h} <<
    std::endl;
}

TEST_F(Variable, variable) {
  variable_all();
}
