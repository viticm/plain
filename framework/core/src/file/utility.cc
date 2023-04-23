#include "plain/file/utility.h"
#include <cassert>
#include "plain/basic/utility.h"

using namespace plain;

FileAppend::FileAppend(const std::string_view &filename)
  : fp_{fopen(filename.data(), "ae")}, buffer_{0}, written_bytes_{0} {
  assert(fp_);
  ::setbuffer(fp_, buffer_, sizeof buffer_);
}

FileAppend::~FileAppend() {
  if (fp_) ::fclose(fp_);
}

void FileAppend::append(const std::string_view &log) {
  std::size_t written{0};
  while (written != log.size()) {
    std::size_t remain = log.size() - written;
    std::size_t n = write({log.data() + written, remain});
    if (n != remain) {
      int32_t err = ferror(fp_);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_pl(err));
        break;
      }
    }
    written += n;
  }

  written_bytes_ += written;
}

void FileAppend::flush() {
  ::fflush(fp_);
}

std::size_t FileAppend::write(const std::string_view &log) {
#if OS_UNIX
  return ::fwrite_unlocked(log.data(), 1, log.size(), fp_);
#else
  return ::fwrite(log.data(), 1, log.size(), fp_);
#endif
}
