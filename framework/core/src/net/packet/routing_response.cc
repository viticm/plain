#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/engine/kernel.h"
#include "pf/basic/type/variable.h"
#include "pf/script/interface.h"
#include "pf/net/connection/basic.h"
#include "pf/net/packet/routing_response.h"

using namespace pf_net::packet;

bool RoutingResponse::read(pf_net::stream::Input &istream) {
  istream.read_string(destination_, sizeof(destination_) - 1);
  istream.read_string(aim_name_, sizeof(aim_name_) - 1);
  return true;
}

bool RoutingResponse::write(pf_net::stream::Output &ostream) {
  ostream << destination_;
  ostream << aim_name_;
  return true;
}

uint32_t RoutingResponse::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(aim_name_);
  result += sizeof(uint32_t);
  result += strlen(destination_);
  return static_cast<uint32_t>(result);
}

uint32_t RoutingResponse::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  std::string func = GLOBALS["default.script.netrouting"].data;
  std::string aim_name{aim_name_};
  connection->set_routing(aim_name, true);
  auto script = ENGINE_POINTER->get_script();
  if (func != "" && !is_null(script)) {
    type::variable_array_t params;
    params.emplace_back(connection->name());
    std::string _aim_name{aim_name_};
    params.emplace_back(_aim_name);
    std::string destination{destination_};
    params.emplace_back(destination);
    type::variable_array_t results;
    script->call(func, params, results);
  }
  return kPacketExecuteStatusContinue;
}
