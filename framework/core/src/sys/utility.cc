#include "plain/sys/utility.h"
#include "plain/basic/utility.h"

namespace plain {

int32_t exec(
    [[maybe_unused]] const char *command,
    [[maybe_unused]] char *result,
    [[maybe_unused]] size_t size) {
#if OS_UNIX
  char buffer[1024] = {0};
  char temp[1024] = {0};
  safecopy(temp, command, sizeof(temp));
  FILE *fp = popen(temp, "r");
  if (!fp) return -1;
  if (fgets(buffer, 1024, fp) != 0) {
    safecopy(result, buffer, size);
  }
  if (fp) pclose(fp);
  fp = nullptr;
#endif
  return 0;
}

bool set_core_rlimit() {
  bool result = true;
#if OS_UNIX
  struct rlimit rlimit_core;
  rlimit_core.rlim_cur = RLIM_INFINITY;
  rlimit_core.rlim_max = RLIM_INFINITY;
  result = 0 == setrlimit(RLIMIT_CORE, &rlimit_core) ? true : false;
#endif
  return result;
}

std::string hostname() {
  char buf[256]{0};
  if (::gethostname(buf, sizeof buf) == 0) {
    buf[sizeof(buf)-1] = '\0';
    return buf;
  } else {
    return "unknownhost";
  }
}

}
