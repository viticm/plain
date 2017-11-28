#include "pf/basic/logger.h"
#include "pf/util/random.h"

std::unique_ptr< pf_util::RandomGenerator > g_random_generator{nullptr};

template<> pf_util::RandomGenerator 
  *pf_basic::Singleton< pf_util::RandomGenerator >::singleton_ = nullptr;

namespace pf_util {

RandomGenerator *RandomGenerator::getsingleton_pointer() {
  return singleton_;
}

RandomGenerator &RandomGenerator::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

RandomGenerator::RandomGenerator(uint32_t seed) {
    srand(seed);
}

RandomGenerator::~RandomGenerator() {
  //do nothing
}

uint32_t RandomGenerator::randuint32() {
  uint32_t a = (rand() & 0x00000FFF);
  uint32_t b = ((rand() & 0x00000FFF) << 12);
  uint32_t c = ((rand() & 0x000000FF) << 24);
  uint32_t d = a + b + c;
  return d;
}

double RandomGenerator::randdouble() {
  double result = 
    static_cast<double>(randuint32()) / static_cast<double>(kRandmonMax);
  return result;
}

uint32_t RandomGenerator::getrand(int32_t start, int32_t end) {
  Assert((end - start + 1) > 0);
  if (!RANDOM_GENERATOR_POINTER) {
    SLOW_ERRORLOG("util", 
                  "[util] RandomGenerator::getrand error, "
                  "RANDOM_GENERATOR_POINTER is NULL");
    return 0;
  };
  uint32_t result = 
    (RANDOM_GENERATOR_POINTER->randuint32() % (end - start + 1)) + start;
  return result;
}

} //namespace pf_util
