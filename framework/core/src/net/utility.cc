#include "plain/net/utility.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/connection/epoll.h"
#include "plain/net/connection/select.h"
#include "plain/net/connection/iocp.h"
#include "plain/net/connection/io_uring.h"
#include "plain/net/connection/kqueue.h"

namespace plain::net {

std::shared_ptr<connection::Manager>
make_manager(
  const setting_t &setting,
  std::shared_ptr<concurrency::executor::Basic> executor) noexcept {
  switch (setting.mode) {
    case Mode::Epoll:
      return std::make_shared<connection::Epoll>(setting, executor);
    case Mode::Select:
      return std::make_shared<connection::Select>(setting, executor);
    case Mode::Iocp:
      return std::make_shared<connection::Iocp>(setting, executor);
    case Mode::IoUring:
      return std::make_shared<connection::IoUring>(setting, executor);
    case Mode::Kqueue:
      return std::make_shared<connection::Kqueue>(setting, executor);
    default:
      return {};
  }
}

} // namespace plain::net
