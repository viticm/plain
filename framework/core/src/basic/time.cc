#include "plain/basic/time.h"

using namespace plain;

Time::Time() : 
  s_time_{std::chrono::steady_clock::now()} {
  // do nothing
}

Time::~Time() = default;


