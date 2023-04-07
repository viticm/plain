/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id async_logger.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/07 18:29
 * @uses The async logger implemantion class.
 */

#ifndef PLAIN_BASIC_ASYNC_LOGGER_H_
#define PLAIN_BASIC_ASYNC_LOGGER_H_

#include "plain/basic/config.h"
#include <string_view>

namespace plain {

class PLAIN_API AsyncLogger : noncopyable {

 public:
  AsyncLogger(const std::string &name,
              off_t roll_size,
              int32_t flush_interval = 3);
  ~AsyncLogger();

 public:
  void append(const std::string_view& log);
  void start();
  void stop();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

};

} // namespace plain

#endif // PLAIN_BASIC_ASYNC_LOGGER_H_
