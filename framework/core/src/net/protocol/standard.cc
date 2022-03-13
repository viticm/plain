#include "pf/basic/string.h"
#include "pf/net/stream/input.h"
#include "pf/net/stream/output.h"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/protocol/standard.h"

using namespace pf_basic::string;
using namespace pf_net::protocol;

bool Standard::command(connection::Basic *connection, uint16_t count) {
  UNUSED(count);
  if (connection->is_disconnect()) return false; //Leave this when not connected.
  stream::Input *istream = &connection->istream();
  auto line = istream->readline();
  if (!line.empty()) {
    if (!connection->check_safe_encrypt()) return false;
    auto listener = dynamic_cast<pf_net::connection::manager::Listener *>(
        connection->get_manager());
    if (!is_null(listener) && listener->is_service()) {
      rtrim(line); // Remove '\n' '\r' or other words on last.
      auto callback = listener->get_standard_callback();
      if (callback) callback(line, connection);
    }
    /**
    std::cout << "line: " << line << std::endl;
    auto ostream = &connection->ostream();
    std::string res{"hello\n"};
    ostream->write(res.c_str(), res.size());
    **/
  }
  return true;
}

bool Standard::send(connection::Basic *connection, packet::Interface *packet) {
  UNUSED(connection);
  UNUSED(packet);
  return true;
}
