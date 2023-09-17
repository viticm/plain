#include "gtest/gtest.h"
#include "plain/basic/logger.h"
#include "plain/basic/async_logger.h"
#include "plain/basic/time.h"
#include "plain/sys/process.h"

using namespace std::chrono_literals;
const uint32_t kRollSize{500*1000*1000};

class AsyncLogger : public testing::Test {

 public:
  static void SetUpTestCase() {
    //Normal.
  }

  static void TearDownTestCase() {
    //std::cout << "TearDownTestCase" << std::endl;
  }

 public:

  virtual void SetUp() {
    plain::Logger::set_output([this](const std::string_view& log) {
      log_.append(log);
    });
    log_.start();
  }

  virtual void TearDown() {
    plain::Logger::set_output([](const std::string_view& log){

    });
  }

 private:
  plain::AsyncLogger log_{"test", kRollSize};

};

void bench(bool long_log) {
  plain::Logger::set_level(plain::LogLevel::Trace);
  // LOG_SYSFATAL << "sysfatal log";
  
  const int kBatch = 1000;
  int cnt = 0;
  std::string s_long(3000, 'X');
  std::string s_empty{" "};
  s_long += " ";

  for (int t = 0; t < 30; ++t) {
    auto start = plain::Time::nanoseconds();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << (long_log ? s_long : s_empty)
               << cnt;
      ++cnt;
    }
    auto end = plain::Time::nanoseconds();
    printf("%ldns\n", (end - start) / kBatch);
    std::this_thread::sleep_for(500ms);
  }
}

TEST_F(AsyncLogger, bench) {
  using namespace plain;
  std::cout << "current pid: " << process::getid() << std::endl;
  // bench(false);
}
