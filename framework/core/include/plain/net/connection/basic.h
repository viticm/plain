/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id basic.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/22 18:08
 * @uses The net connection basic class implemention.
 */

#ifndef PLAIN_NET_CONNECTION_BASIC_H_
#define PLAIN_NET_CONNECTION_BASIC_H_

#include "plain/net/connection/config.h"
#include "plain/net/packet/basic.h"
#include "plain/net/stream/codec.h"
#include "plain/net/rpc/packer.h"
#include "plain/net/rpc/unpacker.h"

namespace plain::net {
namespace connection {

class PLAIN_API alignas(kCacheInlineAlignment)
Basic final : noncopyable {

 public:
  Basic();
  Basic(std::shared_ptr<socket::Basic> socket);
  Basic(Basic &&object) noexcept;
  ~Basic();

 public:
  Basic &operator=(Basic &&object) noexcept;

 public:
  bool init() noexcept;
  bool idle() const noexcept;
  bool shutdown(int32_t how = 0x2) noexcept; // default: SHUT_RDWR = 2
  bool close() noexcept;
  bool valid() const noexcept;

 public:
  id_t id() const noexcept;
  std::string name() const noexcept;
  std::shared_ptr<socket::Basic> socket() const noexcept;
  void set_name(std::string_view name) noexcept;

 public:
  void set_codec(const stream::codec_t &codec) noexcept;
  void set_connect_callback(callable_func func) noexcept;
  void set_disconnect_callback(callable_func func) noexcept;
  void set_dispatcher(packet::dispatch_func func) noexcept;

 public:
  bool send(const std::shared_ptr<packet::Basic> &packet) noexcept;

 public:
  template <typename ...Args>
  rpc::Unpacker call(std::string_view name, Args ...args) {
    auto future = async_call(name, std::forward<Args>(args)...);
    if (auto timeout = get_call_timeout()) {
      auto wait_result = future.wait_for(std::chrono::milliseconds(*timeout));
      if (wait_result == std::future_status::timeout) {
        std::string func_name(name.data(), name.size());
        throw std::logic_error(
          "call " + func_name + " timeout: " + std::to_string(*timeout));
      }
    }
    return future.get();
  }
  template <typename ...Args>
  std::future<rpc::Unpacker> async_call(std::string_view name, Args ...args) {
    auto packet = std::make_shared<packet::Basic>();
    packet->set_writeable(true);
    packet->set_id(packet::kRpcRequestId);
    auto args_tuple = std::make_tuple(args...);
    auto index = new_call_index();
    *(packet) << index;
    *(packet) << name;
    rpc::Packer packer;
    packer.process(args_tuple);
    *(packet) << packer.vector();
    
    auto promise = std::make_shared<std::promise<rpc::Unpacker>>();
    std::string func_name(name.data(), name.size());
    auto future = promise->get_future();
    bool r = send_call(packet, index, func_name, promise);
    try {
      if (!r) {
        throw std::logic_error("async_call " + func_name + " failed");
      }
    } catch(...) {
      promise->set_exception(std::current_exception());
    }
    return future;
  }
  void set_call_timeout(const std::chrono::milliseconds &value) noexcept;
  void clear_call_timeout() noexcept;

 private:
  friend class Manager;
  friend class Epoll;
  friend class Select;
  friend class Iocp;
  friend class IoUring;
  friend class Kqueue;
  friend class net::Connector;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

 private:
  void on_connect() noexcept;
  void on_disconnect() noexcept;

 private:
  void set_id(id_t id) noexcept;
  void set_manager(std::shared_ptr<Manager> manager) noexcept;
  void set_keep_alive(bool flag) const noexcept;
  bool is_keep_alive() const noexcept;

 private:
  bool work() noexcept;
  void enqueue_work(WorkFlag flag) noexcept;

 private:
  bool rpc_dispatch(std::shared_ptr<packet::Basic> packet);
  bool send_call(
    const std::shared_ptr<packet::Basic> &packet,
    uint32_t index, const std::string &func_name,
    std::shared_ptr<std::promise<rpc::Unpacker>> promise);
  uint32_t new_call_index() noexcept;
  std::optional<int64_t> get_call_timeout() const noexcept;

};

} // namespace connection
} // namespace plain::net

#endif // PLAIN_NET_CONNECTION_BASIC_H_
