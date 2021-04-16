#include "pf/basic/string.h"
#include "pf/basic/type/variable.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/engine/kernel.h"
#include "pf/script/interface.h"
#include "pf/net/connection/basic.h"
#include "pf/net/packet/routing_lost.h"

using namespace pf_net::packet;

bool RoutingLost::read(pf_net::stream::Input &istream) {
  istream.read_string(aim_name_, sizeof(aim_name_) - 1);
  return true;
}

bool RoutingLost::write(pf_net::stream::Output &ostream) {
  ostream << aim_name_;
  return true;
}

uint32_t RoutingLost::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(aim_name_);
  return static_cast<uint32_t>(result);
}

uint32_t RoutingLost::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  using namespace pf_basic::type;
  std::string aim_name{aim_name_};
  connection->set_routing(aim_name, false);

  // Handle script function.
  auto script = ENGINE_POINTER->get_script();
  if (!is_null(script) && GLOBALS["default.script.netlost"] != "") {
    auto func = GLOBALS["default.script.netlost"].data;
    variable_array_t params;
    params.emplace_back(aim_name);
    variable_array_t results;
    script->call(func, params, results);
  }

  return kPacketExecuteStatusContinue;
}
