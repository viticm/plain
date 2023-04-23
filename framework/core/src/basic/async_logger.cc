#include "plain/basic/async_logger.h"
#include <latch>
#include <cassert>
#include <filesystem>
#include <string_view>
#include "plain/basic/logger.h"
#include "plain/basic/constants.h"
#include "plain/basic/ring.h"
#include "plain/basic/time.h"
#include "plain/file/utility.h"
#include "plain/sys/thread.h"
#include "plain/sys/process.h"

using namespace plain;
using namespace std::chrono_literals;
using Buffer = FixedRing<char, detail::consts::kLargeBufferSize>;
using BufferVector = std::vector<std::unique_ptr<Buffer>>;
using BufferPtr = BufferVector::value_type;

// FIXME: move this to utility 
std::string get_filename(const std::string &name) {
  std::string r;
  auto log_directory = GLOBALS["log.directory"].data;
  r.reserve(log_directory.length() + 1 + name.length() + 64);
  r = log_directory + "/" + name;
  r += "." + Time::format() + "." + process::hostname();
  r += "." + std::to_string(process::getid()) + ".log";
  return r;
}

// Logger IO file.
// FIXME: this construct can flat few parameters.
struct File {
  File(const std::string &name,
       std::size_t roll_size,
       bool thread_safe = true,
       time_t flush_interval = 3,
       uint32_t check_every_N = 1024);
  ~File() = default;
  void append(const std::string_view &log);
  void append_unlocked(const std::string_view &log);
  void flush();
  bool roll();
  static const int32_t kRollPerSeconds_ = 24 * 60 * 60;
  const std::string name_;
  const std::size_t roll_size_;
  std::unique_ptr<std::mutex> mutex_;
  const time_t flush_interval_;
  const uint32_t check_every_N_;
  std::unique_ptr<FileAppend> file_;
  uint32_t count_;
  time_t start_of_period_;
  time_t last_roll_;
  time_t last_flush_;
};

File::File(const std::string &name,
           std::size_t roll_size,
           bool thread_safe,
           time_t flush_interval,
           uint32_t check_every_N)
  : name_{name}, roll_size_{roll_size},
    mutex_{thread_safe ? std::make_unique<std::mutex>() : nullptr},
    flush_interval_{flush_interval}, check_every_N_{check_every_N},
    file_{nullptr},
    count_{0}, start_of_period_{0}, last_roll_{0}, last_flush_{0} {
  assert(std::string::npos == name.find("/"));
  roll();
}

void File::append(const std::string_view &log) {
  if (mutex_) {
    std::unique_lock<std::mutex> auto_lock{*mutex_};
    append_unlocked(log);
  } else {
    append_unlocked(log);
  }
}

void File::append_unlocked(const std::string_view &log) {
  file_->append(log);

  if (file_->written_bytes() > roll_size_) {
    roll();
  } else {
    ++count_;
    if (count_ >= check_every_N_) {
      count_ = 0;
      auto now = Time::timestamp();
      time_t this_period = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (this_period) {
        roll();
      } else if (now - last_flush_ > flush_interval_) {
        last_flush_ = now;
        file_->flush();
      }
    }
  }
}

void File::flush() {
  if (mutex_) {
    std::unique_lock<std::mutex> auto_lock{*mutex_};
    file_->flush();
  } else {
    file_->flush();
  }
}

bool File::roll() {
  auto now = Time::timestamp();
  auto filename = get_filename(name_);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
  if (now > last_roll_) {
    last_roll_ = now;
    last_flush_ = now;
    start_of_period_ = start;
  
    file_.reset(new FileAppend(filename));
    return true;
  }
  return false;
}

struct AsyncLogger::Impl {
  Impl(const std::string &name, std::size_t roll_size, int32_t flush_interval);
  ~Impl();
  void start();
  void stop();
  void append(const std::string_view &log);
  void thread_handle();
  const std::string name_;
  const std::size_t roll_size_;
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
                        std::size_t roll_size,
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

void AsyncLogger::Impl::append(const std::string_view &log) {
  std::unique_lock<std::mutex> auto_lock{mutex_};
  if (current_buffer_->write_avail() > log.size()) {
    current_buffer_->write(log.data(), log.size());
  } else {
    buffers_.push_back(std::move(current_buffer_));
    if (next_buffer_) {
      current_buffer_ = std::move(next_buffer_);
    } else {
      current_buffer_.reset(new Buffer);
    }
    current_buffer_->write(log.data(), log.size());
    cond_.notify_one();
  }
}

void AsyncLogger::Impl::thread_handle() {
  assert(true == running_);
  File file(name_, roll_size_, false);
  BufferPtr buffer1{std::make_unique<Buffer>()};
  BufferPtr buffer2{std::make_unique<Buffer>()};
  BufferVector write_buffers;
  write_buffers.reserve(16);
  while (running_) {
    assert(buffer1 && 0 == buffer1->read_avail());
    assert(buffer2 && 0 == buffer2->read_avail());
    {
      std::unique_lock<std::mutex> auto_lock{mutex_};
      if (buffers_.empty()) {
        cond_.wait_for(auto_lock, flush_interval_ * 1000ms);
      }
      buffers_.emplace_back(std::move(current_buffer_));
      current_buffer_ = std::move(buffer1);
      write_buffers.swap(buffers_);
      if (nullptr == next_buffer_) {
        next_buffer_ = std::move(buffer2);
      }
    }
    assert(!write_buffers.empty());
    if (write_buffers.size() > 25) {
      char buf[256]{0};
      auto size = snprintf(buf,
                           sizeof buf,
                           "Dropped log messages at %s, %zd larger buffers\n",
                           Time::format(true).c_str(),
                           write_buffers.size() - 2);
      io_cerr(buf);
      file.append({buf, static_cast<std::size_t>(size)});
      write_buffers.erase(write_buffers.begin() + 2, write_buffers.end());
    }
    for (const auto& buf : write_buffers) {
      file.append({buf->peek(), buf->read_avail()});
    }
    if (write_buffers.size() > 2) {
      write_buffers.resize(2);
    }
    if (is_null(buffer1)) {
      assert(!write_buffers.empty());
      buffer1 = std::move(write_buffers.back());
      buffer1->producer_clear();
      write_buffers.pop_back();
    }
    if (is_null(buffer2)) {
      assert(!write_buffers.empty());
      buffer2 = std::move(write_buffers.back());
      buffer2->producer_clear();
      write_buffers.pop_back();
    }
    write_buffers.clear();
    file.flush();
  }
  // Thread end flush.
  file.flush();
}

AsyncLogger::AsyncLogger(const std::string &name, 
                         std::size_t roll_size,
                         int32_t flush_interval)
  : impl_{std::make_unique<Impl>(name, roll_size, flush_interval)} {
  auto log_directory = GLOBALS["log.directory"].data;
  if (!std::filesystem::exists(log_directory)) {
    assert(std::filesystem::create_directory(log_directory));
  }
}

AsyncLogger::~AsyncLogger() = default;

void AsyncLogger::start() {
  impl_->start();
}

void AsyncLogger::stop() {
  impl_->stop();
}

void AsyncLogger::append(const std::string_view &log) {
  impl_->append(log);
}
