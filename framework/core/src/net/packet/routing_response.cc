#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/net/connection/basic.h"
#include "pf/net/packet/routing_response.h"

using namespace pf_net::packet;

bool RoutingResponse::read(pf_net::stream::Input &istream) {
  istream.read_string(aim_name_, sizeof(aim_name_) - 1);
  return true;
}

bool RoutingResponse::write(pf_net::stream::Output &ostream) {
  ostream << aim_name_;
  return true;
}

uint32_t RoutingResponse::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(aim_name_);
  return static_cast<uint32_t>(result);
}

uint32_t RoutingResponse::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  std::string aim_name{aim_name_};
  connection->set_routing(aim_name, true);
  return kPacketExecuteStatusContinue;
}
