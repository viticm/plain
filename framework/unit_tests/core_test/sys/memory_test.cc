#include "gtest/gtest.h"
#include "pf/sys/memory/pool.h"
#include "pf/sys/memory/stack_allocator.h"

using namespace pf_sys::memory;

/* Adjust these values depending on how much you trust your computer */
#define ELEMS 1000000
#define REPS 50

void stack_compare() {
  clock_t start;

  std::cout << "Provided to compare the default allocator to MemoryPool.\n\n";

  /* Use the default allocator */
  StackAllocator<int, std::allocator<int> > stack_default;
  start = clock();
  for (int j = 0; j < REPS; j++) {
    assert(stack_default.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stack_default.push(i);
      stack_default.push(i);
      stack_default.push(i);
      stack_default.push(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stack_default.pop();
      stack_default.pop();
      stack_default.pop();
      stack_default.pop();
    }
  }
  std::cout << "Default Allocator Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  /* Use MemoryPool */
  StackAllocator<int, Pool<int> > stack_pool;
  start = clock();
  for (int j = 0; j < REPS; j++) {
    assert(stack_pool.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stack_pool.push(i);
      stack_pool.push(i);
      stack_pool.push(i);
      stack_pool.push(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stack_pool.pop();
      stack_pool.pop();
      stack_pool.pop();
      stack_pool.pop();
    }
  }
  std::cout << "MemoryPool Allocator Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";


  std::cout << "Here is a secret: the best way of implementing a stack"
            " is a dynamic array.\n";

  /* Compare MemoryPool to std::vector */
  std::vector<int> stack_vector;
  start = clock();
  for (int j = 0; j < REPS; j++) {
    assert(stack_vector.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stack_vector.push_back(i);
      stack_vector.push_back(i);
      stack_vector.push_back(i);
      stack_vector.push_back(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stack_vector.pop_back();
      stack_vector.pop_back();
      stack_vector.pop_back();
      stack_vector.pop_back();
    }
  }
  std::cout << "Vector Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  std::cout << "The vector implementation will probably be faster.\n\n";
}

class Memory : public testing::Test {

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

TEST_F(Memory, testStackCompare) {
  stack_compare();
}
