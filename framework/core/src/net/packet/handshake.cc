#include "pf/basic/base64.h"
#include "pf/basic/string.h"
#include "pf/basic/time_manager.h"
#include "pf/net/packet/handshake.h"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/manager/listener.h"

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
  result += strlen(key_);
  return static_cast<uint32_t>(result);
}

uint32_t Handshake::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  auto listener = connection->get_listener();
  if (!listener) return kPacketExecuteStatusError;
  std::string encrypt_str = listener->get_safe_encrypt_str();
  if ("" == encrypt_str) return kPacketExecuteStatusError;
  std::string key = get_key();
  char decode_key[NET_PACKET_HANDSHAKE]{0};
  pf_basic::base64decode(decode_key, key.c_str());  
  auto now = TIME_MANAGER_POINTER->get_ctime();
  std::string decode_key_1{""};
  int32_t time{0};
  if (now - time > 10 || now - time < 0) return kPacketExecuteStatusError;
  string::decrypt(decode_key, time, decode_key_1);
  if (encrypt_str == decode_key) {
    connection->set_safe_encrypt(true);
    return kPacketExecuteStatusContinue;
  }
  std::string decode_key_2;
  string::decrypt(decode_key_1, decode_key_2);
  if (encrypt_str != decode_key_2) {
    return kPacketExecuteStatusError;
  }
  return kPacketExecuteStatusContinue;
}
