#include "pf/basic/string.h"
#include "pf/sys/assert.h"
#include "pf/util/compressor/mini.h"

using namespace pf_basic;
using namespace pf_util::compressor;

Mini::Mini() 
  : work_memory_{0},
  decompress_buffer_{0},
  decompress_buffersize_{0} {
}

Mini::~Mini() {
  //do nothing
}

bool Mini::init() {
  if (lzo_init() != LZO_E_OK) return false;
  return true;
}

bool Mini::compress(const unsigned char *in,
                    uint32_t insize,
                    unsigned char *out,
                    uint64_t &outsize) {
  if (outsize < UTIL_COMPRESSOR_MINI_GET_OUTLENGTH(insize)) return false;
  int32_t result = lzo1x_1_compress(in,
                                    static_cast<lzo_uint>(insize),
                                    out,
                                    reinterpret_cast<lzo_uintp>(&outsize),
                                    work_memory_);
  if (result != LZO_E_OK) return false;
  return true;
}

bool Mini::compress_andescape(const unsigned char *in,
                              uint32_t insize,
                              unsigned char *out,
                              uint32_t outsize) {
  uint64_t compresssize = UTIL_COMPRESSOR_MINI_GET_OUTLENGTH(insize);
  unsigned char *compressbuffer = 
    new unsigned char[static_cast<size_t>(compresssize)];
  if (nullptr == compressbuffer) return false;
  if (!compress(in, insize, compressbuffer, compresssize)) {
    safe_delete_array(compressbuffer);
    Assert(false);
    return false;
  }
  if (outsize < 2 * compresssize) {
    safe_delete_array(compressbuffer);
    Assert(false);
    return false;
  }
  string::getescape(reinterpret_cast<const char *>(compressbuffer),
                    static_cast<size_t>(compresssize),
                    reinterpret_cast<char *>(out),
                    outsize);
  safe_delete_array(compressbuffer);
  return true;
}

bool Mini::decompress(const unsigned char *in, uint32_t insize) {
  if (0 == insize) {
    decompress_buffersize_ = 0;
    return true;
  }
  int32_t result = lzo1x_decompress(
      in,
      static_cast<lzo_uint>(insize),
      decompress_buffer_,
      reinterpret_cast<lzo_uintp>(&decompress_buffersize_),
      nullptr);
  if (result != LZO_E_OK) return false;
  return true;
}

const unsigned char *Mini::get_decompress_buffer() {
  return decompress_buffer_;
}

uint32_t Mini::get_decompress_buffersize() const {
  uint32_t result = static_cast<uint32_t>(decompress_buffersize_);
  return result;
}
