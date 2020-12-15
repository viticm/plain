#include "pf/basic/base64.h"
#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/basic/time_manager.h"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/packet/handshake.h"

using namespace pf_net::packet;

bool Handshake::read(pf_net::stream::Input &istream) {
  istream.read_string(key_, sizeof(key_) - 1);
  return true;
}

bool Handshake::write(pf_net::stream::Output &ostream) {
  ostream << key_;
  return true;
}

uint32_t Handshake::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(key_);
  return static_cast<uint32_t>(result);
}

uint32_t Handshake::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  auto listener = connection->get_listener();
  if (!listener) return kPacketExecuteStatusError;
  std::string encrypt_str = listener->get_safe_encrypt_str();
  if ("" == encrypt_str) {
    io_cwarn("[%s] The handshake isn't a encrypt connection!",
             NET_MODULENAME);    
    return kPacketExecuteStatusError;
  }
  std::string key = get_key();
  //std::cout << "execute key: " << key << std::endl;
  auto decode_key = pf_basic::base64_decode(key.c_str());  
  auto now = TIME_MANAGER_POINTER->get_ctime();
  std::string decode_key_1{""};
  int32_t time{0};
  string::decrypt(decode_key, time, decode_key_1);
  /**
  std::cout << "encrypt_str: " << encrypt_str 
    << " decode_key: " << decode_key << 
     " decode_key_1: " << decode_key_1 <<
     " time: " << time <<
    std::endl;
  **/
  if (now - time > 10 || (int32_t)now - time < 0) {
    io_cwarn("[%s] The handshake timeout!",
             NET_MODULENAME);    
    return kPacketExecuteStatusError;
  }
  if (encrypt_str == decode_key_1) {
    connection->set_safe_encrypt(true);
    return kPacketExecuteStatusContinue;
  }
  std::string decode_key_2;
  string::decrypt(decode_key_1, decode_key_2);
  if (encrypt_str != decode_key_2) {
    io_cwarn("[%s] The handshake check key failed!",
             NET_MODULENAME);    
    return kPacketExecuteStatusError;
  }
  return kPacketExecuteStatusContinue;
}
