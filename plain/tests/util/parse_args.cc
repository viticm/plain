#include "util/parse_args.h"
#include "plain/basic/io.h"
#include "plain/basic/type/variable.h"

using namespace plain;

bool parse_net_setting(
  plain::net::setting_t &setting, int32_t argc, char **argv) noexcept {
  if (argc < 2) {
    io_cerr("Usage: {} <addr> <mode> <max_count> <sock_type>", argv[0]);
    io_cdebug("addr: string-> ip_v4:port|[ip_v6]:port[not empty]");
    io_cdebug("mode: int-> "
      "select(0)|epoll(1)|io_uring(2)|iocp(3)|kqueue(4)[default:0]");
    io_cdebug("max_count: int-> [default 1024](not active on select mode)");
    io_cdebug("sock_type: int-> tcp(0)|udp(1)[default:0]");
    return false;
  }
  std::string addr = argv[1];
  if (addr.empty()) {
    io_cerr("The addr is empty");
    return false;
  }
  setting.address = addr;
  if (argc >= 3) {
    variable_t temp = argv[2];
    setting.mode = static_cast<net::Mode>(temp.to_int64());
  }
  if (argc >= 4) {
    variable_t temp = argv[3];
    setting.max_count = static_cast<uint32_t>(temp.to_int64());
  }
  if (argc >= 5) {
    variable_t temp = argv[4];
    setting.socket_type = static_cast<net::socket::Type>(temp.to_int64());
  }
  return true;
}
