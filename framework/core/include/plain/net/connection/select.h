/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id select.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/01/04 17:09
 * @uses The select for connection implemention.
 */

#ifndef PLAIN_NET_CONNECTION_SELECT_H_
#define PLAIN_NET_CONNECTION_SELECT_H_

#include "plain/net/connection/config.h"
#include "plain/net/connection/manager.h"

namespace plain::net {
namespace connection {

class Select : public Manager {

 public:
  Select(const setting_t &setting);
  Select(
    std::unique_ptr<concurrency::executor::Basic> &&executor,
    const setting_t &setting);
  virtual ~Select();

 protected:
  bool work() noexcept override;
  void off() noexcept override;
  bool sock_add(
    socket::id_t sock_id, connection::id_t conn_id) noexcept override;
  bool sock_remove(socket::id_t sock_id) noexcept override;
  bool prepare() noexcept override;

 private:
  struct Impl;
  std::shared_ptr<Impl> impl_;

 private:
  void handle_io() noexcept;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_SELECT_H_
