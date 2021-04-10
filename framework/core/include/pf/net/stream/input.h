/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id inputstream.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/20 11:45
 * @uses socket inputstream class
 */
#ifndef PF_NET_STREAM_INPUTSTREAM_H_
#define PF_NET_STREAM_INPUTSTREAM_H_

#include "pf/net/packet/interface.h"
#include "pf/net/stream/basic.h"

namespace pf_net {

namespace stream {

class PF_API Input : public Basic {

 public: //construct and destruct
   Input(
       socket::Basic *_socket, 
       uint32_t bufferlength = NETINPUT_BUFFERSIZE_DEFAULT, 
       uint32_t bufferlength_max = NETINPUT_DISCONNECT_MAXSIZE)
     : Basic(_socket, bufferlength, bufferlength_max), 
     decrypt_buffer_{nullptr} {}
   virtual ~Input() { safe_delete_array(decrypt_buffer_); }
   
 public:
   uint32_t read(char *buffer, uint32_t length);
   //bool readpacket(packet::Base *packet); change this to protocol.
   bool peek(char *buffer, uint32_t length);
   bool skip(uint32_t length);
   int32_t fill();
   // Read a line from stream.
   std::string readline() {
     char buffer[512]{0};
     auto size = get_line_size();
     read(buffer, size);
     return buffer;
   }

 public:
   int8_t read_int8();
   uint8_t read_uint8();
   int16_t read_int16();
   uint16_t read_uint16();
   int32_t read_int32();
   uint32_t read_uint32();
   int64_t read_int64();
   uint64_t read_uint64();
   void read_string(char *buffer, size_t size);
   float read_float();
   double read_double();
   uint32_t read_bytes(unsigned char *buffer, size_t size);

 public:
   //some useful.
   Input &operator >> (bool &var) {
     var = 1 == read_int8() ? true : false;
     return *this;
   };
   Input &operator >> (int8_t &var) {
     var = read_int8();
     return *this;
   };
   Input &operator >> (uint8_t &var) {
     var = read_uint8();
     return *this;
   };
   Input &operator >> (int16_t &var) {
     var = read_int16();
     return *this;
   };
   Input &operator >> (uint16_t &var) {
     var = read_uint16();
     return *this;
   };
   Input &operator >> (int32_t &var) {
     var = read_int32();
     return *this;
   };
   Input &operator >> (uint32_t &var) {
     var = read_uint32();
     return *this;
   };
   Input &operator >> (int64_t &var) {
     var = read_int64();
     return *this;
   };
   Input &operator >> (uint64_t &var) {
     var = read_uint64();
     return *this;
   };
   Input &operator >> (char *&var) { //Not safe.
     uint32_t _size = read_uint32();
     read(var, _size);
     return *this;
   };
   Input &operator >> (std::string &var) { //Need optimize
     uint32_t _size = read_uint32();
     auto temp = new char[_size];
     memset(temp, 0, _size);
     read(temp, _size);
     var = temp;
     safe_delete(temp);
     return *this;
   };

 public: //compress and encrypt mode
   void compressenable(bool enable);
   uint32_t write(const char *buffer, uint32_t length); //why? not receive just copy from memory
                                                        //outputstream have same name functions

 private:

   char *get_decrypt_buffer() {
     if (is_null(decrypt_buffer_)) {
       decrypt_buffer_ = new char[streamdata_.bufferlength_max];
       memset(decrypt_buffer_, 0, streamdata_.bufferlength_max);
     }
     return decrypt_buffer_;
   }
   uint32_t get_line_size();

 private:

   char *decrypt_buffer_;

};

} //namespace socket

} //namespace pf_net

#endif //PF_NET_STREAM_INPUTSTREAM_H_
