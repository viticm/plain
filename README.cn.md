# Plain #

## Plain framework 一个基于C++的网络应用框架 ##

- **作者:** Viticm
- **网站:** [http://www.cnblogs.com/lianyue/](http://www.cnblogs.com/lianyue/)
- **版本:** 2.0.0(C++23 没用使用模块(modules))

[![Build Status](https://travis-ci.org/viticm/plain.svg)](https://travis-ci.org/viticm/plain)

![img](https://github.com/viticm/plain-simple/blob/master/docs/pf-simple.gif)

设计的核心为 `SFE`，即安全、快速、简单.

**我可以做什么:**

1. 安全

你可以加密你的网络连接，用以保护你的应用.

2. 快速

丰富有用的接口让开发来的很快，只需要配置默认的环境配置就可以开启你的应用，同时可
以体验由多线程带来的高性能。

3. 简单

所有接口具有高耦合性，容易使用。你只需要编写自己所需的插件，即可定制丰富的内容。

----------

## 怎么开始使用 plain framework? ##

在UNIX相关的系统中你需要使用以下命令安装g++和cmake:

```shell
git clone --recursive https://github.com/viticm/plain && cd plain
cd cmake && cmake ./ && sudo make install
```

Windows(vs 2022 以上版本可以直接在工程中右键cmake/CMakeList.txt生成).

更新子模块.

```shell
git submodule update --remote --recursive
```


## 定制你自己的网络. ##

```cpp
#include "plain/all.h"

using namespace plain::net;

std::shared_ptr<Listener> listener;
std::shared_ptr<Connector> connector;

// The packet decode function.
// Return a packet shared pointer or error.
plain::error_or_t<std::shared_ptr<packet::Basic>>
line_decode(
  stream::Basic *input, const packet::limit_t &packet_limit) {
  using namespace plain;
  if (!input) return Error{ErrorCode::RunTime};
  bytes_t bytes;
  bytes.reserve(packet_limit.max_length);
  auto readed = input->peek(bytes.data(), bytes.capacity());
  if (readed == 0) return ErrorCode{ErrorCode::NetPacketNeedRecv};
  std::string_view str{reinterpret_cast<char *>(bytes.data()), readed};
  auto pos = str.find('\n');
  if (pos == std::string::npos) {
    if (readed == packet_limit.max_length )
      return Error{ErrorCode::RunTime};
    else
      return Error{ErrorCode::NetPacketNeedRecv};
  }
  input->remove(pos + 1); // remove readed line.
  if (pos > 0 && str[pos - 1] == '\r') {
    pos -= 1;
  }
  auto p = std::make_shared<packet::Basic>();
  if (pos > 0) {
    p->set_writeable(true);
    p->write(bytes.data(), pos);
    p->set_writeable(false);
  }
  p->set_readable(true);
  return p;
}

// The packet encode function.
// Return a byte string.
plain::bytes_t line_encode(std::shared_ptr<packet::Basic> packet) {
  auto d = packet->data();
  return {d.data(), d.size()};
}

bool start_listener() {

  setting_t setting;
  setting.max_count = 512; // The connection max count
  setting.default_count = 32; // The connection object default created
  // The mode can be: select/epoll/iocp/iouring/kqueue
  // select is default mode(all platform can be supported).
  // epoll just active on linux.
  // iocp just active on windows.
  // iouring just active on linux kernel 5.0+.
  // kqueue just active on macos.
  setting.mode = Mode::Select; 
  setting.socket_type = socket::Type::Tcp; // Tcp/Udp
  setting.address = ":2001"; // Just active with Listener(empty system divide a port)
  setting.name = "listener"; // Empty kernel will divide a unknown name
  // The max packet id(if you custom packet handler this need check by yourself)
  setting.packet_limit.max_id = 65535;
  // The max packet length
  // (if you custom packet handler this need check by yourself)
  setting.packet_limit.max_length = 200 * 1024;

  listener = std::make_shared<Listener>(setting);

  // The codec handlers.
  listener->set_codec({.encode = line_encode, .decode = line_decode});

  // Set the packet dispachter.
  listener->set_dispatcher([](
    connection::Basic *conn, std::shared_ptr<packet::Basic> packet) {
    std::cout << conn->name() << ": " <<
      reinterpret_cast<const char *>(packet->data().data()) << std::endl;
    return true;
  });

  // Set the callback on connected.
  listener->set_connect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " connected" << std::endl;
  });

  // Set the callback on disconnected.
  listener->set_disconnect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " disconnected" << std::endl;
  });

  return listener->start();
}

void start_connector() {
  
  connector = std::make_shared<Connector>(); // Using default setting.
  connector->set_codec({.encode = line_encode, .decode = line_decode});
  connector->set_dispatcher([](
    connection::Basic *conn, std::shared_ptr<packet::Basic> packet) {
    std::cout << conn->name() << ": " <<
      reinterpret_cast<const char *>(packet->data().data()) << std::endl;
    return true;
  });

  connector->set_connect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " connected" << std::endl;
  });

  connector->set_disconnect_callback([](connection::Basic *conn) {
    std::cout << conn->name() << " disconnected" << std::endl;
  });

  if (!connector->start()) return;

  auto pack = std::make_shared<packet::Basic>();
  pack->set_writeable(true);
  std::string line{"hello world\n"};
  pack->write(reinterpret_cast<std::byte *>(line.data()), line.size());
  std::string line1{"plain\n"};
  pack->write(reinterpret_cast<std::byte *>(line1.data()), line1.size());
  auto conn = connector->connect(":2001");
  
  if (static_cast<bool>(conn)) {
    conn->send(pack);
  }
}

int32_t main(int32_t argc, char **argv) {
  using namespace std::chrono_literals;
  
  // Start console, param empty then listen a random port.
  if (!ENGINE->enable_console(":3001"))
    return 1;
  if (!start_listener())
    return 1;
  start_connector();

  // Wait exit.
  while (listener->running()) {
    std::this_thread::sleep_for(100ms);
  }
  return 0;
}
```

## 简单的项目. ##

很多开发者如果初次接触会对框架感到疑惑，因此可以参考该项目快速上手.

项目地址:

[https://github.com/viticm/plain-simple](https://github.com/viticm/plain-simple)
