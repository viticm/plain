#include "plain/concurrency/executor/basic.h"

namespace plain::concurrency {
namespace executor::detail {

void throw_runtime_shutdown_exception(std::string_view executor_name) {
  const auto error_msg = std::string(executor_name) +
    " - shutdown has been called on this executor.";
  throw std::runtime_error(error_msg);
}

std::string make_executor_worker_name(std::string_view executor_name) {
  return std::string(executor_name) + " worker";
}

} // namespace executor::detail
} // namespace plain::concurrency
