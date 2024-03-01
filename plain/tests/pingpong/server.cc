#include "util/parse_args.h"
#include "util/net.h"
#include "plain/engine/timer.h"
#include "plain/net/connection/basic.h"
#include "plain/net/listener.h"

using namespace plain;

int32_t main(int32_t argc, char **argv) {
  net::setting_t setting;
  setting.name = "server";
  if (!parse_net_setting(setting, argc, argv))
    return 1;
  auto listener = std::make_shared<net::Listener>(setting);
  if (!listener->start())
    return 1;
  listener->set_dispatcher([](
    net::connection::Basic *conn, std::shared_ptr<net::packet::Basic> pack) {
    // std::cout << "send: " << conn->name() << std::endl;
    conn->send(pack);
    return true;
  });
  listener->set_disconnect_callback([](net::connection::Basic *conn) {
    std::cout << conn->name() << " disconnect" << std::endl;
  });
  auto timer = counter_net(listener);
  UNUSED(timer);
  wait_shutdown(listener);
  return 0;
}
