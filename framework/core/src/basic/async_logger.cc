#include "plain/basic/async_logger.h"
#include <latch>
#include <cassert>
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
  void thread_handle();
  void start();
  void stop();
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
  thread_t thread_;
};

AsyncLogger::Impl::Impl(const std::string &name,
                        off_t roll_size,
                        int32_t flush_interval)
  : name_{name}, roll_size_{roll_size}, flush_interval_{flush_interval},
    mutex_{}, running_{false}, latch_{1}, cond_{},
    current_buffer_{std::make_unique<Buffer>()},
    next_buffer_{std::make_unique<Buffer>()}, buffers_{} {

  buffers_.reserve(16);
}

AsyncLogger::Impl::~Impl() {
  if (running_) stop();
}

void AsyncLogger::Impl::start() {
  assert(!latch_.try_wait());
  running_ = true;
  thread_ = thread::create("async_logger", [this]() {
    latch_.count_down(); // Why use this latch ? thread func failed with except
    thread_handle();
  });
  latch_.wait();
}

void AsyncLogger::Impl::stop() {
  running_ = false;
  cond_.notify_one();
  thread_.join();
}

void AsyncLogger::Impl::thread_handle() {
  assert(true == running_);
  latch_.count_down();

}

AsyncLogger::AsyncLogger(const std::string &name, 
                         off_t roll_size,
                         int32_t flush_interval)
  : impl_{std::make_unique<Impl>(name, roll_size, flush_interval)} {
  // do nothing
}

AsyncLogger::~AsyncLogger() = default;
