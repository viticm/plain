#include "pf/util/compressor/minimanager.h"
#include "pf/util/compressor/assistant.h"

using namespace pf_util::compressor;

Assistant::Assistant()
  : workmemory_{nullptr},
  isenable_{false},
  log_isenable_{false},
  compressframe_{0},
  compressframe_success_{0} {
}

Assistant::~Assistant() {
  //do nothing
}

void Assistant::enable(bool _enable, uint64_t threadid) {
  if (true == _enable) {
    if (!UTIL_COMPRESSOR_MINIMANAGER_POINTER ||
        nullptr == UTIL_COMPRESSOR_MINIMANAGER_POINTER->alloc(threadid)) {
      isenable_ = false;
    }
  }
  isenable_ = _enable;
}
