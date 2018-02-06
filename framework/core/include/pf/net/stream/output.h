/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id outputstream.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/21 12:03
 * @uses socket output stream class
 */
#ifndef PF_NET_STREAM_OUTPUTSTREAM_H_
#define PF_NET_STREAM_OUTPUTSTREAM_H_

#include "pf/net/packet/interface.h"
#include "pf/net/stream/basic.h"

namespace pf_net {

namespace stream {

class PF_API Output : public Basic {

 public:
   Output(
     socket::Basic *_socket, 
       uint32_t bufferlength = NETOUTPUT_BUFFERSIZE_DEFAULT,
       uint32_t bufferlength_max = NETOUTPUT_DISCONNECT_MAXSIZE)
     : Basic(_socket, bufferlength, bufferlength_max), tail_(0) {};
   virtual ~Output() {};

 public:
   void clear();

 public:
   uint32_t write(const char *buffer, uint32_t length);
   //bool writepacket(packet::Base *packet); change this to protocol.
   int32_t flush();

 public: //write_*常用方法
   bool write_int8(int8_t value);
   bool write_uint8(uint8_t value);
   bool write_int16(int16_t value);
   bool write_uint16(uint16_t value);
   bool write_int32(int32_t value);
   bool write_uint32(uint32_t value);
   bool write_int64(int64_t value);
   bool write_uint64(uint64_t value);
   bool write_string(const char *value);
   bool write_float(float value);
   bool write_dobule(double value);

 public:
   Output &operator << (bool var) {
     write_int8(var ? 1 : 0);
     return *this;
   };
   Output &operator << (int8_t var) {
     write_int8(var);
     return *this;
   };
   Output &operator << (uint8_t var) {
     write_uint8(var);
     return *this;
   };
   Output &operator << (int16_t var) {
     write_int16(var);
     return *this;
   };
   Output &operator << (uint16_t var) {
     write_uint16(var);
     return *this;
   };
   Output &operator << (int32_t var) {
     write_int32(var);
     return *this;
   };
   Output &operator << (uint32_t var) {
     write_uint32(var);
     return *this;
   };
   Output &operator << (int64_t var) {
     write_int64(var);
     return *this;
   };
   Output &operator << (uint64_t var) {
     write_uint64(var);
     return *this;
   };
   Output &operator << (const char *var) {
     write_string(var);
     return *this;
   };
   Output &operator << (const std::string &var) {
     auto length = var.size();
     if (!write_uint32(length)) return *this;
     write(var.c_str(), length);
     return *this;
   };

 private: //compress mode is enable, use this functions replace normals.
   uint32_t get_floortail();
   void compressenable(bool enable);
   bool compress(uint32_t tail);
   int32_t compressflush();
   int32_t rawflush();
   bool raw_isempty() const;
   void rawprepare(uint32_t tail);


 private:
   uint32_t tail_; //compress mode is enable, tail_ will replace streamdata.tail

};

} //namespace stream

} //namespace pf_net


#endif //PF_NET_STREAM_OUTPUTSTREAM_H_
