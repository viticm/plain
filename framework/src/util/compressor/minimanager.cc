#include "pf/sys/assert.h"
#include "pf/util/compressor/minimanager.h"

using namespace pf_util::compressor;

pf_util::compressor::MiniManager *g_util_compressor_minimanager = nullptr;

template <>
pf_util::compressor::MiniManager
  *pf_basic::Singleton<pf_util::compressor::MiniManager>::singleton_ = nullptr;

MiniManager *MiniManager::getsingleton_pointer() {
  return singleton_;
}

MiniManager &MiniManager::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

MiniManager::MiniManager()
  : log_isenable_{false},
  workmemory_size_{0},
  uncompress_size_{0},
  compress_size_{0} {
  for (uint16_t i = 0; i < UTIL_COMPRESSOR_MINI_MANAGER_THREAD_SIZEMAX; ++i) {
    workmemory_[i].pointer = nullptr;
    workmemory_[i].threadid = 0;
  }
}

MiniManager::~MiniManager() {
  destory();
}

bool MiniManager::init() {
  bool result = LZO_E_OK == lzo_init();
  return result;
}

void MiniManager::destory() {
  workmemory_size_ = 0;
  uncompress_size_ = 0;
  compress_size_ = 0;
  for (uint16_t i = 0; i < UTIL_COMPRESSOR_MINI_MANAGER_THREAD_SIZEMAX; ++i) {
    char *pointer = reinterpret_cast<char *>(workmemory_[i].pointer);
    safe_delete_array(pointer);
    workmemory_[i].threadid = 0;
  }
}

void *MiniManager::alloc(uint64_t threadid) {
  std::unique_lock<std::mutex> autolock(mutex_);
  for (uint16_t i = 0; i < UTIL_COMPRESSOR_MINI_MANAGER_THREAD_SIZEMAX; ++i) {
    if (workmemory_[i].threadid == threadid) return workmemory_[i].pointer;
  }
  if (workmemory_size_ > UTIL_COMPRESSOR_MINI_MANAGER_THREAD_SIZEMAX) 
    return nullptr;
  lzo_align_t* workmemory = 
    new lzo_align_t[UTIL_COMPRESSOR_MINI_MANAGER_WORK_MEMORY_SIZE];
  workmemory_[workmemory_size_].pointer = workmemory;
  workmemory_[workmemory_size_].threadid = threadid;
  ++workmemory_size_;
  return workmemory;
}

bool MiniManager::compress(const unsigned char *in,
                           uint32_t insize,
                           unsigned char *out,
                           uint32_t &outsize,
                           void *workmemory) {
  int32_t result = 
    lzo1x_1_compress(in, insize, out, (lzo_uint *)&outsize, workmemory);
  if (result != LZO_E_OK || outsize + 2 >= insize) return false;
  return true;
}

int32_t MiniManager::decompress(const unsigned char *in,
                                uint32_t insize,
                                unsigned char *out,
                                uint32_t &outsize) {
  int32_t result = 
    lzo1x_decompress(in, insize, out, (lzo_uint *)&outsize, nullptr);
  return result;
}
