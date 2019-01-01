#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/net/connection/basic.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/packet/forward.h"

using namespace pf_net::packet;

bool Forward::read(pf_net::stream::Input &istream) {
  istream.read_string(original_, sizeof(original_) - 1);
  packet_size_ = istream.read_uint32();
  return true;
}

bool Forward::write(pf_net::stream::Output &ostream) {
  ostream << original_;
  ostream << packet_size_;
  return true;
}

uint32_t Forward::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(original_);
  result += sizeof(uint32_t);
  return static_cast<uint32_t>(result);
}

uint32_t Forward::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  //std::cout << "Forward::execute" << std::endl;
  std::string original{original_};
  auto protocol = connection->protocol();
  auto packet = protocol->read_packet(connection);
  if (is_null(packet)) return kPacketExecuteStatusError;
  if (packet->size() != packet_size_) {
    io_cwarn("[%s] Forward the packet size error(%d|%d)!",
             NET_MODULENAME,
             packet->size(),
             packet_size_);    
    NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
    return kPacketExecuteStatusError;
  }
  //std::cout << "Forward: " << packet->get_id() << " original: " << original << std::endl;
  NET_PACKET_FACTORYMANAGER_POINTER->packet_execute(connection, packet, original);
  NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
  return kPacketExecuteStatusContinue;
}
