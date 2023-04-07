#include "plain/basic/async_logger.h"
#include <latch>
#include "plain/basic/constants.h"
#include "plain/basic/ring.h"
#include "plain/sys/thread.h"

using namespace plain;
using Buffer = FixedRing<char, detail::consts::kLargeBufferSize>;
using BufferVector = std::vector<std::unique_ptr<Buffer>>;
using BufferPtr = BufferVector::value_type;

struct AsyncLogger::Impl {
  Impl(const std::string& name, off_t roll_size, int32_t flush_interval);
  ~Impl();
  bool thread_handle();
  const std::string name_;
  const off_t roll_size_;
  const int32_t flush_interval_;
  std::mutex mutex_;
  std::atomic<bool> running_;
  std::latch latch_;
  std::condition_variable cond_;
  BufferPtr current_buffer_;
  BufferPtr next_buffer_;
  BufferVector buffers_;
  std::thread thread_;
};

std::thread gen_thread() {
  return std::thread([] {std::cout << "gen_thread" << std::endl;});
}

AsyncLogger::Impl::Impl(const std::string &name,
                        off_t roll_size,
                        int32_t flush_interval)
  : name_{name}, roll_size_{roll_size}, flush_interval_{flush_interval},
    mutex_{}, running_{false}, latch_{1}, cond_{}, current_buffer_{nullptr},
    next_buffer_{nullptr}, buffers_{} {
  thread_ = thread::create("async_logger", [this]() {
    return thread_handle();
  });
  buffers_.reserve(16);
}

AsyncLogger::Impl::~Impl() {
  thread_.join();
}

bool AsyncLogger::Impl::thread_handle() {
  return true;
}
