/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id process.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/03 11:45
 * @uses The system process module
 */
#ifndef PLAIN_SYS_PROCESS_H_
#define PLAIN_SYS_PROCESS_H_

#include "plain/sys/config.h"
#include "plain/basic/io.h"

namespace plain::process {

typedef struct {
  int32_t id;
  float cpu_percent;
  uint64_t VSZ;
  uint64_t RSS;
} info_t;

PLAIN_API int32_t getid();
PLAIN_API int32_t getid(const char *filename);
PLAIN_API bool writeid(const char *filename);
PLAIN_API bool waitexit(const char *filename);
PLAIN_API void get_info(int32_t id, info_t &info);
PLAIN_API void get_filename(char *filename, size_t size);
PLAIN_API float get_cpu_usage(int32_t id);
PLAIN_API uint64_t get_virtualmemory_usage(int32_t id);
PLAIN_API uint64_t get_physicalmemory_usage(int32_t id);
PLAIN_API bool daemon();
PLAIN_API std::string hostname();

inline void print_curinfo() {
  io_cdebug(
    "cpu: {}% VSZ: {}k RSS: {}k", get_cpu_usage(getid()),
    get_virtualmemory_usage(getid()), get_physicalmemory_usage(getid()));
}

} //namespace plain::process

#endif //PLAIN_SYS_PROCESS_H_
