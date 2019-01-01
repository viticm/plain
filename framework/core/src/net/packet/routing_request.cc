#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/engine/kernel.h"
#include "pf/net/packet/routing_request.h"
#include "pf/net/packet/routing_response.h"

using namespace pf_net::packet;

bool RoutingRequest::read(pf_net::stream::Input &istream) {
  istream.read_string(destination_, sizeof(destination_) - 1);
  istream.read_string(aim_name_, sizeof(aim_name_) - 1);
  aim_id_ = istream.read_uint16();
  return true;
}

bool RoutingRequest::write(pf_net::stream::Output &ostream) {
  ostream << destination_;
  ostream << aim_name_;
  ostream << aim_id_;
  return true;
}

uint32_t RoutingRequest::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(destination_);
  result += sizeof(uint32_t);
  result += strlen(aim_name_);
  result += sizeof(uint16_t);
  return static_cast<uint32_t>(result);
}

uint32_t RoutingRequest::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  auto listener = connection->get_listener();
  std::string destination{destination_};
  std::string aim_name{aim_name_};
  if (!listener) return kPacketExecuteStatusContinue;
  /**
  std::cout << "RoutingRequest-> " << listener->name() << "destination: " 
            << destination << " aim_name: " << aim_name << std::endl;
  **/
  if (connection->name() == "") return kPacketExecuteStatusContinue;
  auto destination_service = ENGINE_POINTER->get_service(destination);
  if (is_null(destination_service)) {
    io_cwarn("[%s] Routing request can't get service from(%s)!",
             NET_MODULENAME,
             destination.c_str());    
    return kPacketExecuteStatusContinue;
  }
  if (!is_null(destination_service->get(aim_name))) {
    io_cwarn("[%s] Routing request connection has in service(%s)!",
             NET_MODULENAME,
             aim_name.c_str());    
    return kPacketExecuteStatusContinue;
  }
  auto destination_connection = destination_service->get(aim_id_);
  if (is_null(destination_connection) || destination_connection->name() != "") {
    io_cwarn("[%s] Routing request connection disconnect or has name(%d)!",
             NET_MODULENAME,
             aim_id_);    
    return kPacketExecuteStatusContinue;
  }
  //Success.
  destination_service->set_connection_name(aim_id_, aim_name);
  destination_connection->set_name(aim_name);
  if (listener->name() != "") 
    destination_connection->set_param("routing_service", listener->name());
  destination_connection->set_param("routing", connection->name());
  RoutingResponse r;
  r.set_destination(destination);
  r.set_aim_name(aim_name);
  connection->send(&r);
  return kPacketExecuteStatusContinue;
}
