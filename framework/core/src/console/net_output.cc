#include "pf/net/connection/basic.h"
#include "pf/console/net_output.h"

using namespace pf_console;

void NetOutput::write(
    const std::string &messages, bool newline, uint16_t options) {
  if (is_null(connection_) || connection_->is_disconnect()) return;
  std::string str = newline ? messages + "\n" : messages;
  // std::cout << "messages: " << str << std::endl;
  auto ostream = &connection_->ostream();
  ostream->write(str.c_str(), str.size());
}
