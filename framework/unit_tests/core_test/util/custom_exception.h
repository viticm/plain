#ifndef PLAIN_CORE_TEST_CUSTOM_EXCEPTION_H_
#define PLAIN_CORE_TEST_CUSTOM_EXCEPTION_H_

#include <cstdint>
#include <memory>
#include <numeric>
#include <exception>

namespace plain::tests {

struct custom_exception : public std::exception {
  const intptr_t id;

  custom_exception(intptr_t id) noexcept : id(id) {}
};

}  // namespace plain::tests

#endif
