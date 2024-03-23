#include "gtest/gtest.h"
#include "plain/basic/logger.h"

class Logger : public testing::Test {

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

void logger_all() {
  plain::Logger::set_level(plain::LogLevel::Trace);
  LOG_TRACE << "trace log";
  LOG_DEBUG << "debug log";
  LOG_INFO << "info log";
  LOG_WARN << "warn log";
  LOG_ERROR << "error log";
  LOG_SYSERR << "syserr log";
  // LOG_SYSFATAL << "sysfatal log";
}

TEST_F(Logger, loggerAll) {
  logger_all();
}
