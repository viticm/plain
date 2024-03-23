#include "gtest/gtest.h"
#include "plain/sys/memory/stack_allocator.h"
#include "plain/sys/memory/pool.h"

using namespace plain::memory;
using TestFun = std::function<bool(int cnt, std::size_t each_size)>;

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

bool test_OS_malloc(int cnt, std::size_t each_size) {
  bool re = true;
  std::vector<void *> vec_allocs;
  vec_allocs.reserve(cnt);
  for (int i = 0; i < cnt; ++i) {
    void *ptr = malloc(each_size);
    if (ptr) {
      vec_allocs.emplace_back(ptr);
    }
  }
  for (auto &&it : vec_allocs) {
    free(it);
  }
  return re;
} 

bool test_mempool(int cnt, std::size_t each_size) {
  using namespace plain::memory;
  bool re = true;
  //std::vector<void *> vec_allocs;
  //vec_allocs.reserve(cnt);

  // Pool Constructor will takes time here:
  Pool<16000000, 16, 16, false, false> mem_pool;
  for (int i = 0; i < cnt; ++i) {
    void *ptr = mem_pool.malloc(each_size);
    UNUSED(ptr);
//    if (ptr)
//    {
//      all allocations in Pool, no need vec_allocs for free
//      vec_allocs.emplace_back(ptr);
//    }
  }

//  for (auto &&it : vec_allocs) {
//      all allocations in Pool, no need vec_allocs for free
//    free(it);
//  }

  // Pool Destructor will takes time here..
  return re;
}

TEST_F(Memory, testPool) {
  int threads_cnt = 2;
  /*
  if (argc > 1) {
    threads_cnt  = static_cast<int>(
    stoll(argv[1], static_cast<int>(strlen(argv[1]))));
    if (!threads_cnt)  threads_cnt  =  2;
  }
  */
  // std::cout << "\nMemory overhead for each allocation bytes=" << 
  // sizeof (Pool<>::AllocHeader) << std::endl;
  struct AllocHeader {
    /*
     label of own allocations:
     tag_this = (uint64_t)this + leaf_id */
    uint64_t tag_this{ 2020071700 };
    // allocation size (without sizeof(AllocHeader)):
    int32_t size;
    // allocation place id (Leaf ID or OS_malloc_id):
    int32_t leaf_id{ -2020071708 };
  };
  std::cout << "\nMemory overhead for each allocation bytes=" 
    << sizeof(AllocHeader) << std::endl;
  // For the convenience of a random choice, 
  // we will emplace these methods into a vector:

  std::map<std::string, TestFun> map_fun;

  std::cout << "\nMulti threaded (threads =" 
    <<  threads_cnt  << "), msec for each count:";
  map_fun.emplace("|  test_mempool               ", test_mempool);
  map_fun.emplace("|  test_OS_malloc             ", test_OS_malloc);
  std::cout 
    << 
    "\n-----------------------------------------------------------------------"
    "----------"            
    << 
    "\n|  test name, msec for allocs:|\t1000|\t10000|\t100000|\t1000000|"
    << 
    "\n-----------------------------------------------------------------------"
    "----------";

  for (auto &&it : map_fun) {
    std::cout << std::endl << it.first << "|\t";
    for (int32_t cnt = 1000; cnt < 1000001; cnt *= 10) {
      std::vector<std::thread> vec_threads;
      int64_t start = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
      for (int32_t n = 0; n < threads_cnt; ++n) {
        //vec_threads.emplace_back(run_TestFun(it.second,  cnt,  256));
        vec_threads.emplace_back(it.second,  cnt,  256);
      }
      // Wait:
      for (auto &it2 : vec_threads) {
        it2.join();
      }
      int64_t end = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
      std::cout << (end - start) << "|\t";
    }
  }
  std::cout
    << 
    "\n-----------------------------------------------------------------------"
    "----------\n";
}
