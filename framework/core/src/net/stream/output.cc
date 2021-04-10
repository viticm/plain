#include "pf/net/socket/basic.h"
#include "pf/net/stream/output.h"

namespace pf_net {

namespace stream {

void Output::clear() {
  Basic::clear();
  tail_ = 0;
}

uint32_t Output::write(const char *buffer, uint32_t length) {
  /**
   * tail head       head tail --length 10
   * 0123456789      0123456789
   * abcd...efg      ...abcd...
   */
  auto &head = streamdata_.head;
  auto &tail = streamdata_.tail;
  auto &bufferlength = streamdata_.bufferlength;
  size_t freecount{0};
  if (!use(length)) return 0;
  if (head <= tail) {
    freecount = bufferlength - tail - 1;
    if (length <= freecount) {
      if (encrypt_isenable()) {
        encryptor_.encrypt(&(streamdata_.buffer[tail]), buffer, length);
      } else {
        memcpy(&(streamdata_.buffer[tail]), buffer, length);
      }
    } else {
      if (encrypt_isenable()) {
        encryptor_.encrypt(&(streamdata_.buffer[tail]), buffer, freecount);
        encryptor_.encrypt(streamdata_.buffer, 
                           &buffer[freecount], 
                           bufferlength - freecount);
      } else {
        memcpy(&(streamdata_.buffer[tail]), buffer, freecount);
        memcpy(streamdata_.buffer, &buffer[freecount], length - freecount);
      }
    }
  } else {
    if (encrypt_isenable()) {
      encryptor_.encrypt(&(streamdata_.buffer[tail]), buffer, length);
    } else {
      memcpy(&(streamdata_.buffer[tail]), buffer, length);
    }
  }
  streamdata_.tail = (tail + length) % bufferlength;
  return length;
}

int32_t Output::flush() {
  if (!socket_->is_valid()) return 0;
  if (0 == size()) return 0;
  if (compressor_.getassistant()->isenable()) { //compress is enable
    uint32_t result = 0;
    uint32_t sendcount = 0;
    uint32_t tail = get_floortail();
    if (static_cast<int32_t>(tail) < -1) return tail;
    if (compress(tail)) return -1;
    result = compressflush();
    sendcount += result;
    if (static_cast<int32_t>(result) <= SOCKET_ERROR) 
      return static_cast<int32_t>(result);
    rawprepare(tail);
    result = rawflush();
    sendcount += result;
    if (static_cast<int32_t>(result) <= SOCKET_ERROR) 
      return static_cast<int32_t>(result);
    return sendcount;
  }
  uint32_t flushcount = 0;
  int32_t sendcount = 0;
  uint32_t leftcount = 0;
  uint32_t flag = 0;
  if (streamdata_.bufferlength > streamdata_.bufferlength_max) {
    init();
    return SOCKET_ERROR - 1;
  }
#if OS_UNIX
  flag = MSG_NOSIGNAL;
#elif OS_WIN
  flag = MSG_DONTROUTE;
#endif
  leftcount = streamdata_.head < streamdata_.tail ? 
              streamdata_.tail - streamdata_.head : 
              streamdata_.bufferlength - streamdata_.head;
  while (leftcount > 0) {
    sendcount = 
      socket_->send(&streamdata_.buffer[streamdata_.head], leftcount, flag);
    if (SOCKET_ERROR_WOULD_BLOCK == sendcount) {
      return flushcount;
    }
    if (SOCKET_ERROR == sendcount) {
      return SOCKET_ERROR - 2;
    }
    if (0 == sendcount) {
      return flushcount;
    }
    flushcount += sendcount;
    leftcount -= sendcount;
    streamdata_.head += sendcount;
  }
  if (streamdata_.head > streamdata_.tail) {
    streamdata_.head = 0;
    leftcount = streamdata_.tail;
    while (leftcount > 0) {
      sendcount = socket_->send(
          &(streamdata_.buffer[streamdata_.head]), leftcount, flag);
      if (SOCKET_ERROR_WOULD_BLOCK == sendcount) {
        return flushcount;
      }
      if (SOCKET_ERROR == sendcount) {
        return SOCKET_ERROR - 3;
      }
      if (0 == sendcount) {
        return flushcount;
      }
      flushcount += sendcount;
      leftcount -= sendcount;
      streamdata_.head += sendcount;
    }
  }
  if (streamdata_.head == streamdata_.tail)
    streamdata_.head = streamdata_.tail = 0;
  int32_t result = static_cast<int32_t>(flushcount);
  return result;
}

bool Output::write_int8(int8_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_uint8(uint8_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_int16(int16_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_uint16(uint16_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_int32(int32_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_uint32(uint32_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_int64(int64_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
  return false;
}
   
bool Output::write_uint64(uint64_t value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}
   
bool Output::write_string(const char *value) {
  uint32_t count = 0;
  uint32_t length = static_cast<uint32_t>(strlen(value));
  bool result = false;
  if (!write_uint32(length)) return false;
  count = write(value, length);
  result = count == length ? true : false;
  return result;
}
   
bool Output::write_float(float value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_dobule(double value) {
  uint32_t count = write((char*)&value, sizeof(value));
  bool result = count == sizeof(value) ? true : false;
  return result;
}

bool Output::write_bytes(const unsigned char *value, size_t _size) {
  write_uint32(_size);
  return write((const char *)value, _size) == _size;
}

int32_t Output::compressflush() {
  if (0 == compressor_.getsize()) return 0;
  uint32_t flushcount = 0;
  uint32_t leftcount = compressor_.getsize();
  uint32_t sendcount = 0;
  uint32_t flag = 0;
#if OS_UNIX
  flag = MSG_NOSIGNAL;
#elif OS_WIN
  flag = MSG_DONTROUTE;
#endif
  bool totalsend = true;
  if (leftcount > 1024) {
    leftcount = 1024;
    totalsend = false;
  }
  while (leftcount > 0) {
    sendcount = socket_->send(compressor_.getheader(), leftcount, flag);
    if (SOCKET_ERROR_WOULD_BLOCK == static_cast<int32_t>(sendcount)) return 0;
    if (SOCKET_ERROR == static_cast<int32_t>(sendcount)) 
      return SOCKET_ERROR - 12;
    if (0 == sendcount) return 0;
    flushcount += sendcount;
    compressor_.pophead(sendcount);
    leftcount -= sendcount;
  }
  if (!totalsend) return flushcount;
  compressor_.resetposition();
  return flushcount;
}

bool Output::compress(uint32_t tail) {
  if (static_cast<uint32_t>(-1) == tail ||
      compressor_.getsize() != 0 ||
      size() <= 0) {
    return false;
  }
  uint32_t head = streamdata_.head;
  uint32_t _tail = streamdata_.tail;
  uint32_t bufferlength = streamdata_.bufferlength;
  uint32_t bufferlength_max = streamdata_.bufferlength_max;
  if (bufferlength > bufferlength_max) return false;
  compressor_.resetposition();
  char *inbuffer = nullptr;
  uint32_t insize = _tail - head;
  uint32_t outsize = 0;
  if (insize < NET_STREAM_COMPRESSOR_SIZE_MIN) return false;
  inbuffer = streamdata_.buffer + head;
  bool compress_result = 
    compressor_.compress(inbuffer, insize, compressor_.getheader(), outsize);
  if (!compress_result) return false;
  if (encrypt_isenable()) compressor_.encrypt();
  streamdata_.head = (head + insize) % bufferlength;
  if (streamdata_.head == streamdata_.tail)
    streamdata_.head = streamdata_.tail = 0;
  return true;
}

uint32_t Output::get_floortail() {
  uint32_t result = 0;
  uint32_t head = streamdata_.head;
  uint32_t tail = streamdata_.tail;
  uint32_t bufferlength = streamdata_.bufferlength;
  uint32_t bufferlength_max = streamdata_.bufferlength_max;
  if (bufferlength > bufferlength_max) {
    result = static_cast<uint32_t>(SOCKET_ERROR - 21);
    return result;
  }
  if (compressor_.getsize() != 0 || !raw_isempty() || 0 == size()) {
    result = static_cast<uint32_t>(SOCKET_ERROR);
    return result;
  }
  uint32_t position = head;
  uint32_t _size{0};
  char *buffer = streamdata_.buffer + position;
  uint32_t sizemax = NET_STREAM_COMPRESSOR_IN_SIZE;
  if (size() < sizemax) return tail;
  //uint16_t last_packetid = static_cast<uint16_t>(-1);
  do {
    uint32_t packetcheck = 0;
    uint32_t packetsize = 0;
    uint16_t packetid = 0;
    if (_size + 4 > size()) {
      result = static_cast<uint32_t>(SOCKET_ERROR - 17);
      return result;
    }
    if (encrypt_isenable()) {
      encryptor_.decrypt(&packetid, buffer, sizeof(packetid));
      encryptor_.decrypt(&packetcheck, 
                         buffer + sizeof(packetid), 
                         sizeof(packetcheck));
    } else {
      memcpy(&packetid, buffer, sizeof(packetid));
      memcpy(&packetcheck, buffer + sizeof(packetid), sizeof(packetcheck));
    }
    packetsize = NET_PACKET_GETLENGTH(packetcheck);
    if (packetsize > sizemax) {
      result = static_cast<uint32_t>(SOCKET_ERROR - 15);
      return result;
    }
    uint32_t full_packetsize = 
      sizeof(packetid) + sizeof(packetcheck) + packetsize;
    if (_size + full_packetsize > bufferlength) {
      result = static_cast<uint32_t>(SOCKET_ERROR - 16);
      return result;
    }
    if (_size + full_packetsize > sizemax) {
      break;
    }
    _size += full_packetsize;
    position = (position + full_packetsize) % bufferlength;
    buffer = streamdata_.buffer + position;
    //last_packetid = packetid;
  } while(true);
  return position;
}

void Output::rawprepare(uint32_t tail) {
  if (0 == compressor_.getsize() && 
      raw_isempty() && 
      tail != static_cast<uint32_t>(-1)) {
    tail_ = tail;
  }
}

bool Output::raw_isempty() const {
  return tail_ == streamdata_.head;
}

int32_t Output::rawflush() {
  if (compressor_.getsize() != 0 || raw_isempty()) return 0;
  uint32_t flushcount = 0;
  uint32_t sendcount = 0;
  uint32_t leftcount = 0;
  uint32_t result = 0;
  uint32_t flag = 0;
#if OS_UNIX
  flag = MSG_NOSIGNAL;
#elif OS_WIN
  flag = MSG_DONTROUTE;
#endif
  if (streamdata_.bufferlength > streamdata_.bufferlength_max) {
    result = static_cast<uint32_t>(SOCKET_ERROR - 11);
    return result;
  }
  bool totalsend = true;
  if (streamdata_.head < tail_) {
    leftcount = tail_ - streamdata_.head;
    if (leftcount > 1024) {
      leftcount = 1024;
      totalsend = false;
    }
    while (leftcount > 0) {
      sendcount = 
        socket_->send(&streamdata_.buffer[streamdata_.head], leftcount, flag);
      if (SOCKET_ERROR_WOULD_BLOCK == static_cast<int32_t>(sendcount)) 
        return 0;
      if (SOCKET_ERROR == static_cast<int32_t>(sendcount)) 
        return SOCKET_ERROR - 12;
      if (0 == sendcount) return 0;
      flushcount += sendcount;
      leftcount -= sendcount;
      streamdata_.head += sendcount;
    }
    if (!totalsend) return flushcount;
  } else if (streamdata_.head > tail_) {
    leftcount = streamdata_.bufferlength - streamdata_.head;
    if (leftcount > 1024) {
      leftcount = 1024;
      totalsend = false;
    }
    while (leftcount > 0) {
      sendcount = 
        socket_->send(&streamdata_.buffer[streamdata_.head], leftcount, flag);
      if (SOCKET_ERROR_WOULD_BLOCK == static_cast<int32_t>(sendcount)) 
        return 0;
      if (SOCKET_ERROR == static_cast<int32_t>(sendcount)) 
        return SOCKET_ERROR - 12;
      if (0 == sendcount) return 0;
      flushcount += sendcount;
      leftcount -= sendcount;
      streamdata_.head += sendcount;
    }
    if (!totalsend) {
      return flushcount;
    } else {
      streamdata_.head = 0;
    }
  }
  if (streamdata_.head == streamdata_.tail)
    streamdata_.head = streamdata_.tail = tail_ = 0;
  return flushcount;
}

void Output::compressenable(bool enable) {
  Basic::compressenable(enable);
  if (compressor_.getassistant()->isenable())
    compressor_.alloc(NET_STREAM_COMPRESSOR_OUT_SIZE);
}

} //namespace stream

} //namespace pf_net
