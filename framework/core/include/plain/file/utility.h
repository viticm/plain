/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id utility.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/04/09 14:32
 * @uses The file utility.
 */

#ifndef PLAIN_FILE_UTILITY_H_
#define PLAIN_FILE_UTILITY_H_

#include "plain/file/config.h"
#include <string_view>

namespace plain {

class PLAIN_API FileAppend : noncopyable {

 public:
  explicit FileAppend(const std::string_view &filename);
  ~FileAppend();

 public:
  void append(const std::string_view &log);

  void flush();

  off_t written_bytes() const { return written_bytes_; }

 private:
  std::size_t write(const std::string_view &log);

 private:
  FILE* fp_;
  char buffer_[64 * 1024];
  off_t written_bytes_;

};

} // namespace plain

#endif // PLAIN_FILE_UTILITY_H_
