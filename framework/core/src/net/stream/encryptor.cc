#include "pf/basic/string.h"
#include "pf/net/stream/encryptor.h"

using namespace pf_net::stream;

Encryptor::Encryptor() {
  isenable_ = false;
  memset(key_, 0, sizeof(key_));
}

Encryptor::~Encryptor() {
  //do nothing
}

void *Encryptor::encrypt(void *out, const void *in, uint32_t count) {
  void *result = out;
  while (count--) {
    *reinterpret_cast<char *>(out) = 
      *reinterpret_cast<char *>(const_cast<void *>(in));
    if (isenable()) { //enable with key
      uint8_t low = 0;
      uint8_t high = 0;
      low = (*reinterpret_cast<char *>(out)) & 0x0F;
      high = (*reinterpret_cast<char *>(out)) & 0xF0;
      high = high ^ (key_[low] & 0xF0);
      low = (((low ^ 0x0F) & 0x0F) + (key_[0] & 0x0F)) & 0x0F;
      *reinterpret_cast<char *>(out) = high + low;
    } else {
      *reinterpret_cast<char *>(out) = *reinterpret_cast<char *>(out) ^ 0xFF;
    }
    out = reinterpret_cast<char *>(out) + 1;
    in = reinterpret_cast<char *>(const_cast<void *>(in)) + 1;
  }
  return result;
}

void *Encryptor::decrypt(void *out, const void *in, uint32_t count) {
  void *result = out;
  while (count--) {
    *reinterpret_cast<char *>(out) = 
      *reinterpret_cast<char *>(const_cast<void *>(in));
    if (isenable()) { //enable with key
      uint8_t low = 0;
      uint8_t high = 0;
      low = (*reinterpret_cast<char *>(out)) & 0x0F;
      high = (*reinterpret_cast<char *>(out)) & 0xF0;
      low = ((low - (key_[0] & 0x0F)) & 0x0F) ^ 0x0F;
      high = high ^ (key_[low] & 0xF0);
      *reinterpret_cast<char *>(out) = high + low;
    } else {
      *reinterpret_cast<char *>(out) = *reinterpret_cast<char *>(out) ^ 0xFF;
    }
    out = reinterpret_cast<char *>(out) + 1;
    in = reinterpret_cast<char *>(const_cast<void *>(in)) + 1;
  }
  return result;
}
