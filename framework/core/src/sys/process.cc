#include "plain/sys/process.h"
#if OS_WIN
#include <process.h>
#include <psapi.h>
#elif OS_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#endif
#include "plain/basic/utility.h"
#include "plain/basic/io.h"
#include "plain/sys/utility.h"
#include "plain/sys/assert.h"

namespace plain::process {

int32_t getid() {
  int32_t id = ID_INVALID;
#if OS_WIN
  id = _getpid();
#elif OS_UNIX
  id = getpid();
#endif
  return id;
}

void get_filename(char* filename, size_t size) {
  get_module_filename(filename, size);
  auto havelength = strlen(filename);
  auto _size = size - havelength;
  if (_size > 0) snprintf(filename + havelength, _size, "%s", ".pid");
}

int32_t getid(const char* filename) {
  FILE *fp = fopen(filename, "r");
  if (nullptr == fp) return ID_INVALID;
  int32_t id;
  fscanf(fp, "%d", &id);
  fclose(fp);
  return id;
}

bool writeid(const char* filename) {
  int32_t id = getid();
  FILE *fp = fopen(filename, "w");
  if (nullptr == fp) return false;
  fprintf(fp, "%d", id);
  fflush(fp);
  fclose(fp);
  fp = nullptr;
  return true;
}

bool waitexit(const char* filename) {
  using namespace plain;
  if (nullptr == filename || 0 == strlen(filename)) {
    io_cerr("[sys] (process::waitexit) error, can't find pid file");
    return false;
  }
  int32_t id = getid(filename);
  if (ID_INVALID == id) {
    io_cerr("[sys] (process::waitexit) error, can't get id from file: %s",
            filename);
    return false;
  }
#if OS_UNIX
  kill(id, 10);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  kill(id, 10);
  int32_t result = kill(id, 0);
  while (result >= 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    result = kill(id, 0);
  }
  io_cerr("[sys] (process::waitexit) success, pid(%d), result(%d)", id, result);
#endif
  return true;
}

void getinfo(int32_t id, info_t &info) {
  info.id = id;
  info.cpu_percent = get_cpu_usage(id);
  info.VSZ = get_virtualmemory_usage(id);
  info.RSS = get_physicalmemory_usage(id);
}

#if OS_WIN
static uint64_t file_time_2_utc(const FILETIME* ftime) {
  LARGE_INTEGER li;
  Assert(ftime);
  li.LowPart = ftime->dwLowDateTime;
  li.HighPart = ftime->dwHighDateTime;
  return li.QuadPart;
}

static int32_t get_processor_number() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  int32_t result = static_cast<int32_t>(info.dwNumberOfProcessors);
  return result;
}
#endif 

float get_cpu_usage(int32_t id) {
  float cpu = -1.0f;
#if OS_WIN /* { */
  static int processor_count = -1;
  static int64_t last_time = 0;
  static int64_t last_system_time = 0;
  FILETIME now;
  FILETIME creation_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;
  int64_t system_time;
  int64_t time;
  int64_t system_time_delta;
  int64_t time_delta;
  if (-1 == processor_count) {
    processor_count = get_processor_number();
  }
  GetSystemTimeAsFileTime(&now);
  HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
  if (!::GetProcessTimes(hProcess, 
                         &creation_time, 
                         &exit_time,
                         &kernel_time, 
                         &user_time)) {
    return -1.0f;
  }
  system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / 
    processor_count;
  time = file_time_2_utc(&now);
  if ((0 == last_system_time) || (0 == last_time)) {
    last_system_time = system_time;
    last_time = time;
    return -1.0f;
  }
  system_time_delta = system_time - last_system_time;
  time_delta = time - last_time;
  if (time_delta == 0) return -1.0f;
  cpu = static_cast<float>(
    (system_time_delta * 100 + time_delta / 2) / time_delta);
  last_system_time = system_time;
  last_time = time;
#elif OS_UNIX /* } { */
  char temp[32] = {0};
  char command[128] = {0};
  snprintf(command, 
           sizeof(command) - 1, 
           "ps aux | awk '{if ($2 == %d) print $3}'",
           id);
  if (0 == exec(command, temp, sizeof(temp))) {
    cpu = atof(temp);
  }
#endif /* } */
  return cpu;
}

uint64_t get_virtualmemory_usage(int32_t id) {
    uint64_t result = 0;
#if OS_WIN /* { */
    PROCESS_MEMORY_COUNTERS pmc;
    HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
    if (::GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
      result = pmc.PagefileUsage;
    }
#elif OS_UNIX /* }{ */
  char temp[128] = {0};
  char command[128] = {0};
  snprintf(command, 
           sizeof(command) - 1, 
           "ps aux | awk '{if ($2 == %d) print $5}'",
           id);
  if (0 == exec(command, temp, sizeof(temp))) {
    char* endpointer = nullptr;
    result = strtouint64(temp, &endpointer, 10);
    result *= 1024;
  }
#endif /* } */
    return result;
}

uint64_t get_physicalmemory_usage(int32_t id) {
    uint64_t result = 0;
#if OS_WIN /* { */
    PROCESS_MEMORY_COUNTERS pmc;
    HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
    if (::GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
      result = pmc.WorkingSetSize;
    }
#elif OS_UNIX /* }{ */
  char temp[128] = {0};
  char command[128] = {0};
  snprintf(command, 
           sizeof(command) - 1, 
           "ps aux | awk '{if ($2 == %d) print $6}'",
           id);
  if (0 == exec(command, temp, sizeof(temp))) {
    char* endpointer = nullptr;
    result = strtouint64(temp, &endpointer, 10);
    result *= 1024;
  }
#endif /* } */
  return result;
}

bool daemon() {
  bool result = false;
#if OS_UNIX
  pid_t pid;
  if ((pid = fork()) != 0) exit(0);
  setsid();
  signal(SIGHUP, SIG_IGN);
  if ((pid = fork()) != 0) exit(0);
  umask(0);
  for (uint8_t i = 0; i < 3; i++) close(i);
  result = true;
#endif
  return result;
}

} // namespace plain::process
