#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/engine/kernel.h"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/packet/routing.h"

using namespace pf_net::packet;

bool Routing::read(pf_net::stream::Input &istream) {
  istream.read_string(destination_, sizeof(destination_) - 1);
  istream.read_string(aim_name_, sizeof(aim_name_) - 1);
  packet_size_ = istream.read_uint32();
  return true;
}

bool Routing::write(pf_net::stream::Output &ostream) {
  ostream << destination_;
  ostream << aim_name_;
  ostream << packet_size_;
  return true;
}

uint32_t Routing::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(destination_);
  result += sizeof(uint32_t);
  result += strlen(aim_name_);
  result += sizeof(uint32_t);
  return static_cast<uint32_t>(result);
}

uint32_t Routing::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  std::string aim_name{aim_name_};
  /**
  std::cout << "Routing-> destination: " << destination_ << " aim_name: " 
            << aim_name << std::endl;
  **/
  auto listener = ENGINE_POINTER->get_service(destination_);
  if (is_null(listener)) {
    io_cwarn("[%s] Routing request can't get service from(%s)!",
             NET_MODULENAME,
             destination_);    
    return kPacketExecuteStatusError;
  }
  auto destination_connection = listener->get(aim_name);
  if (is_null(destination_connection)) {
    io_cwarn("[%s] Routing request connection(%s) disconnect!",
             NET_MODULENAME,
             aim_name.c_str());    
    return kPacketExecuteStatusError;
  }
  auto protocol = connection->protocol();
  auto packet = protocol->read_packet(connection);
  if (is_null(packet)) return kPacketExecuteStatusError;
  if (packet->size() != packet_size_) {
    io_cwarn("[%s] Routing the packet size error(%d|%d)!",
             NET_MODULENAME,
             packet->size(),
             packet_size_);    
    NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
    return kPacketExecuteStatusError;
  }
  destination_connection->send(packet);
  NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
  return kPacketExecuteStatusContinue;
}
