#include "pf/net/packet/factorymanager.h"
#include "pf/net/connection/basic.h"
#include "pf/net/packet/interface.h"

namespace pf_net {

namespace packet {

queue_struct::~queue_struct() {
  safe_delete(packet);
}

uint32_t Interface::execute(connection::Basic *connection) {
  uint32_t result = kPacketExecuteStatusError;
  if (!NET_PACKET_FACTORYMANAGER_POINTER) return result;
  result = NET_PACKET_FACTORYMANAGER_POINTER->packet_execute(connection, this);
  return result;
}

} //namespace packet

} //namespace pf_net
