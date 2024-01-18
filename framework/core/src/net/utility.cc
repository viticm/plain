#include "plain/net/utility.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/connection/epoll.h"
#include "plain/net/connection/select.h"
#include "plain/net/connection/iocp.h"
#include "plain/net/connection/io_uring.h"
#include "plain/net/connection/kqueue.h"

namespace plain::net {

std::shared_ptr<connection::Manager>
make_manager(Mode mode, const setting_t &setting) noexcept {
  switch (mode) {
    case Mode::Epoll:
      return std::make_shared<connection::Epoll>(setting);
    case Mode::Select:
      return std::make_shared<connection::Select>(setting);
    case Mode::Iocp:
      return std::make_shared<connection::Iocp>(setting);
    case Mode::IoUring:
      return std::make_shared<connection::IoUring>(setting);
    case Mode::KQueue:
      return std::make_shared<connection::KQueue>(setting);
    default:
      return {};
  }
}

std::shared_ptr<connection::Manager>
make_manager(
  Mode mode, std::unique_ptr<concurrency::executor::Basic> &&executor,
  const setting_t &setting) noexcept {
  switch (mode) {
    case Mode::Epoll:
      return std::make_shared<connection::Epoll>(
        std::forward<decltype(executor)>(executor), setting);
    case Mode::Select:
      return std::make_shared<connection::Select>(
        std::forward<decltype(executor)>(executor), setting);
    case Mode::Iocp:
      return std::make_shared<connection::Iocp>(
        std::forward<decltype(executor)>(executor), setting);
    case Mode::IoUring:
      return std::make_shared<connection::IoUring>(
        std::forward<decltype(executor)>(executor), setting);
    case Mode::KQueue:
      return std::make_shared<connection::KQueue>(
        std::forward<decltype(executor)>(executor), setting);
    default:
      return {};
  }
}

} // namespace plain::net
