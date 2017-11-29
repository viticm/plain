#include "pf/util/compressor/minimanager.h"
#include "pf/net/stream/compressor.h"

using namespace pf_net::stream;

Compressor::Compressor() :
  buffer_{nullptr},
  head_{NET_STREAM_COMPRESSOR_HEADER_SIZE},
  tail_{NET_STREAM_COMPRESSOR_HEADER_SIZE},
  maxsize_{0},
  encryptor_{nullptr} {
}

Compressor::~Compressor() {
  safe_delete_array(buffer_);
  clear();
}

void Compressor::clear() {
  head_ = NET_STREAM_COMPRESSOR_HEADER_SIZE;
  tail_ = NET_STREAM_COMPRESSOR_HEADER_SIZE;
  maxsize_ = 0;
  buffer_ = nullptr;
  encryptor_ = nullptr;
}

bool Compressor::alloc(uint32_t size) {
  safe_delete_array(buffer_);
  buffer_ = new char[size];
  if (is_null(buffer_)) return false;
  return true;
}

char *Compressor::getbuffer() {
  return buffer_;
}

char *Compressor::getheader() {
  return buffer_ + head_;
}

uint32_t Compressor::getsize() const {
  return tail_ - head_;
}

uint32_t Compressor::get_maxsize() const {
  return maxsize_;
}

void Compressor::sethead(uint32_t head) {
  head_ = head;
}

void Compressor::settail(uint32_t tail) {
	tail_ = tail;
}

bool Compressor::pushback(uint32_t size) {
  tail_ += size;
  if (tail_ > maxsize_) {
    tail_ = maxsize_ - 1;
    return false;
  }
  return true;
}

bool Compressor::pophead(uint32_t size) {
  head_ += size;
  if (head_ > tail_) return false;
  return true;
}

void Compressor::add_packetheader() {
  uint16_t size = static_cast<uint16_t>(getsize());
  size |= 0x8000;
  *reinterpret_cast<uint16_t *>(buffer_) = size;
  head_ = 0;
}

void Compressor::encrypt() {
  encryptor_->encrypt(buffer_, buffer_, getsize());
}

void Compressor::resetposition() {
  head_ = tail_ = NET_STREAM_COMPRESSOR_HEADER_SIZE;
}

void Compressor::setencryptor(Encryptor *encryptor) {
  encryptor_ = encryptor;
}

bool Compressor::compress(const char *in, 
                          uint32_t insize, 
                          char *out, 
                          uint32_t &outsize) {
  assistant_.compressframe_inc();
  if (insize < NET_STREAM_COMPRESSOR_IN_SIZE) return false;
  const unsigned char *_in = reinterpret_cast<const unsigned char *>(in);
  unsigned char *_out = reinterpret_cast<unsigned char *>(out);
  bool result = UTIL_COMPRESSOR_MINIMANAGER_POINTER->compress(
      _in, insize, _out, outsize, assistant_.get_workmemory());
  if (true == result) {
    assistant_.compressframe_successinc();
    pushback(outsize);
    add_packetheader();
    //logging
    if (UTIL_COMPRESSOR_MINIMANAGER_POINTER->log_isenable()) {
      UTIL_COMPRESSOR_MINIMANAGER_POINTER->add_compress_datasize(
          outsize + NET_STREAM_COMPRESSOR_HEADER_SIZE);
      UTIL_COMPRESSOR_MINIMANAGER_POINTER->add_uncompress_datasize(insize);
    }
  }
  return result;
}

bool Compressor::decompress(const char *in,
                            uint32_t insize,
                            char *out,
                            uint32_t &outsize) {
  const unsigned char *_in = reinterpret_cast<const unsigned char *>(in);
  unsigned char *_out = reinterpret_cast<unsigned char *>(out);
  int32_t result = UTIL_COMPRESSOR_MINIMANAGER_POINTER->decompress(
      _in + NET_STREAM_COMPRESSOR_HEADER_SIZE, insize, _out, outsize);
  bool _result = LZO_E_OK == result;
  return _result;
}

pf_util::compressor::Assistant *Compressor::getassistant() {
  pf_util::compressor::Assistant *assistant = &assistant_;
  return assistant;
}
