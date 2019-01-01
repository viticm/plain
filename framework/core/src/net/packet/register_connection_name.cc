#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/basic/io.tcc"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/packet/register_connection_name.h"

using namespace pf_net::packet;

bool RegisterConnectionName::read(pf_net::stream::Input &istream) {
  istream.read_string(name_, sizeof(name_) - 1);
  return true;
}

bool RegisterConnectionName::write(pf_net::stream::Output &ostream) {
  ostream << name_;
  return true;
}

uint32_t RegisterConnectionName::size() const {
  size_t result = 0;
  result += sizeof(uint32_t);
  result += strlen(name_);
  return static_cast<uint32_t>(result);
}

uint32_t RegisterConnectionName::execute(pf_net::connection::Basic *connection) {
  using namespace pf_net::connection;
  using namespace pf_basic;
  auto listener = connection->get_listener();
  /**
  std::cout << "RegisterConnectionName" << "connname: " << connection->name() 
            << " name: " << name_ << " listener: " << listener << " id: " 
            << connection->get_id() << std::endl;
  **/
  std::string name{name_};
  if (is_null(listener)) return kPacketExecuteStatusContinue;
  if (connection->name() != "" || name == "") 
    return kPacketExecuteStatusContinue;
  if (!is_null(listener->get(name))) return kPacketExecuteStatusError;
  connection->set_name(name);
  listener->set_connection_name(connection->get_id(), name);
  return kPacketExecuteStatusContinue;
}
