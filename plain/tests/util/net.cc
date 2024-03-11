#include "util/net.h"
#include <format>
#include "plain/basic/rang.h"
#include "plain/basic/rlutil.h"
#include "plain/basic/utility.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/engine/kernel.h"
#include "plain/engine/timer_queue.h"
#include "plain/net/listener.h"
#include "plain/net/connector.h"

using namespace std::chrono_literals;

template <typename T>
plain::Timer counter_net_impl(
  std::shared_ptr<T> manager,
  std::shared_ptr<plain::concurrency::executor::Basic> executor) noexcept {
  using namespace plain::rlutil;
  using namespace plain::rang;
  if (!static_cast<bool>(executor)) {
    executor = manager->get_executor();
  }
  if (!static_cast<bool>(executor)) return {};
  auto timer_queue = ENGINE->timer_queue();
  if (static_cast<bool>(timer_queue)) {
    auto r = timer_queue->make_timer(1s, 1s, executor, [manager]() {
      const CursorHider hider;
      auto send_str = plain::format_size(manager->send_size());
      auto recv_str = plain::format_size(manager->recv_size());
      auto str = std::vformat(
        "send:{}/recv:{}", std::make_format_args(send_str, recv_str));
      str.resize(64);
      std::cout << fgB::green;
      setString(str);
      std::cout.flush();
      std::cout << fg::reset;
    });
    return r;
  }
  return {};
}

plain::Timer counter_net(
  std::shared_ptr<plain::net::Listener> listener,
  std::shared_ptr<plain::concurrency::executor::Basic> executor) noexcept {
  return counter_net_impl(listener, executor);
}

plain::Timer counter_net(
  std::shared_ptr<plain::net::Connector> connector,
  std::shared_ptr<plain::concurrency::executor::Basic> executor) noexcept {
  return counter_net_impl(connector, executor);
}

void wait_shutdown(std::shared_ptr<plain::net::Listener> listener) noexcept {
  while (listener->running()) {
    std::this_thread::sleep_for(100ms);
  }
}

void wait_shutdown(std::shared_ptr<plain::net::Connector> connector) noexcept {
  while (connector->running()) {
    std::this_thread::sleep_for(100ms);
  }
}
