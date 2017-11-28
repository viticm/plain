#include "pf/basic/string.h"
#include "pf/sys/thread.h"
#include "pf/sys/util.h"

namespace pf_sys {

namespace util {

#if OS_WIN
int32_t exec(const char *, char *, size_t) {
#elif OS_UNIX
int32_t exec(const char *command, char *result, size_t size) {
    using namespace pf_basic;
    char buffer[1024] = {0};
    char temp[1024] = {0};
    string::safecopy(temp, command, sizeof(temp));
    FILE *fp = popen(temp, "r");
    if (!fp) return -1;
    if (fgets(buffer, 1024, fp) != 0) {
      string::safecopy(result, buffer, size);
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

} //namespace util

} //namespace pf_sys
