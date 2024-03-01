#include "util/parse_args.h"
#include "util/net.h"
#include "plain/basic/type/variable.h"
#include "plain/engine/kernel.h"
#include "plain/engine/timer.h"
#include "plain/net/connection/basic.h"
#include "plain/net/packet/basic.h"
#include "plain/net/connector.h"

using namespace plain;

int32_t main(int32_t argc, char **argv) {
  net::setting_t setting;
  setting.name = "client";
  if (!parse_net_setting(setting, argc, argv))
    return 1;
  uint32_t block_size{512};
  if (argc >= 6) {
    variable_t temp = argv[5];
    block_size = static_cast<uint32_t>(temp.to_int64());
  }
  std::string str;
  for (uint32_t i = 0; i < block_size; ++i) {
    str.push_back(static_cast<char>(i % 128));
  }
  uint32_t client_count{32};
  if (argc >= 7) {
    variable_t temp = argv[6];
    client_count = static_cast<uint32_t>(temp.to_int64());
  }
  auto connector = std::make_shared<net::Connector>(setting);
  if (!connector->start())
    return 1;

  auto pack = std::make_shared<net::packet::Basic>();
  pack->set_id(1);
  pack->set_writeable(true);
  *pack << str;
  connector->set_dispatcher([](
    net::connection::Basic *conn, std::shared_ptr<net::packet::Basic> pack) {
    // std::cout << "send: " << conn << std::endl;
    conn->send(pack);
    return true;
  });
  connector->set_connect_callback([](net::connection::Basic *conn) {
    std::cout << conn->name() << " connect" << std::endl;
  });

  for (uint32_t i = 0; i < client_count; ++i) {
    auto conn = connector->connect(setting.address);
    if (!conn) break;
    conn->send(pack);
  }
 
  auto timer = counter_net(connector);
  UNUSED(timer);
  wait_shutdown(connector);
 
  return 0;
}
