#include "plain/net/connection/basic.h"
#include <map>
#include "plain/basic/utility.h"
#include "plain/basic/logger.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/detail/coroutine.h"
#include "plain/net/socket/basic.h"
#include "plain/net/stream/basic.h"
#include "plain/net/connection/manager.h"
#include "plain/net/packet/basic.h"
#include "plain/net/rpc/packer.h"
#include "plain/net/rpc/unpacker.h"
#include "plain/net/rpc/dispatcher.h"

using plain::net::connection::Basic;
using plain::net::connection::callable_func;

static std::unordered_map<std::string, callable_func> s_callables;
static constexpr size_t kOnceHandlePacketCount{12};
static constexpr size_t kCantPeekMaxCount{60};

static std::string
get_callable_key(Basic *p, std::string_view suffix) noexcept {
  auto np = (int64_t)(reinterpret_cast<int64_t *>(p));
  return std::to_string(np) + std::string{suffix};
}

static void check_callable(Basic *p, std::string_view suffix) noexcept {
  auto key = get_callable_key(p, suffix);
  auto it = s_callables.find(key);
  if (it != s_callables.end())
    it->second(p);
}

struct Basic::Impl {
  Impl();
  Impl(std::shared_ptr<socket::Basic> socket);
  ~Impl();
  id_t id;
  std::shared_ptr<socket::Basic> socket;
  std::unique_ptr<stream::Basic> istream;
  std::unique_ptr<stream::Basic> ostream;
  stream::codec_t codec;
  std::weak_ptr<Manager> manager;
  packet::dispatch_func dispatcher;
  uint8_t error_times{0};
  uint8_t work_flags{0};
  std::atomic_bool working{false};
  std::atomic_bool keep_alive{false};
  mutable std::mutex mutex;
  
  // For rpc calls.
  using call_t = std::pair<std::string, std::promise<rpc::Unpacker>>;
  std::atomic_uint32_t call_index{0};
  std::unordered_map<uint32_t, call_t> callings;
  std::optional<int64_t> timeout{std::nullopt};

  bool process_input(Basic *conn) noexcept;
  bool process_output(Basic *conn) noexcept;
  net::detail::Task<> process_input_await(Basic *conn) noexcept;
  net::detail::Task<> process_output_await(Basic *conn) noexcept;
  bool process_command(Basic *conn) noexcept;
  bool process_exception() noexcept;
  bool work(Basic *conn) noexcept;
  void enqueue_work() noexcept; 
  bool has_work_flag(WorkFlag flag) const noexcept;
  void set_work_flag(WorkFlag flag, bool enable) noexcept;
  bool exchange_work_flag(WorkFlag flag, bool enable) noexcept;
  static std::string get_name(const Basic *conn) noexcept;
  bool handle_rpc_request(Basic *conn, std::shared_ptr<packet::Basic> packet);
  bool handle_rpc_response(Basic *conn, std::shared_ptr<packet::Basic> packet);
  bool write(const std::shared_ptr<packet::Basic> &packet) noexcept;
};

Basic::Impl::Impl() :
  id{kInvalidId},
  socket{std::make_shared<socket::Basic>()},
  istream{std::make_unique<stream::Basic>(socket)},
  ostream{std::make_unique<stream::Basic>(socket)},
  codec{}, manager{} {

}

Basic::Impl::Impl(std::shared_ptr<socket::Basic> socket) :
  id{kInvalidId},
  socket{socket},
  istream{std::make_unique<stream::Basic>(socket)},
  ostream{std::make_unique<stream::Basic>(socket)},
  codec{}, manager{} {

}

Basic::Impl::~Impl() = default;

bool Basic::Impl::process_input(Basic *conn) noexcept {
  if (!has_work_flag(WorkFlag::Input)) return true;
  scoped_executor_t check_flag([this]{
    if (socket->avail() == 0)
      set_work_flag(WorkFlag::Input, false);
  });
  assert(conn);
  if (!socket->valid()) return false;
  auto r = istream->pull();
  if (r < 0) {
    LOG_ERROR << get_name(conn) << " pull failed: " << r;
    return false;
  }
  auto m = manager.lock();
  if (m) {
    m->increase_recv_size(r);
  }
  set_work_flag(WorkFlag::Command, true);
  /*
  if (socket->avail() == 0)
    set_work_flag(WorkFlag::Input, false);
  */
  return true;
}

bool Basic::Impl::process_output(Basic *conn) noexcept {
  if (!has_work_flag(WorkFlag::Output)) return true;
  assert(conn);
  if (!socket->valid()) return false;
  {
    std::unique_lock<std::mutex> lock{mutex};
    auto r = ostream->push();
    if (r < 0) {
      LOG_ERROR << get_name(conn) << " push failed: " << r;
      return false;
    }
    auto m = manager.lock();
    if (r > 0 && m) {
      m->increase_send_size(r);
    }
  }
  if (ostream->size() == 0)
    set_work_flag(WorkFlag::Output, false);
  return true;
}

plain::net::detail::Task<>
Basic::Impl::process_input_await(Basic *conn) noexcept {
  auto m = manager.lock();
  if (!m) co_return;
  auto sock_data = m->get_sock_data();
  auto r = co_await istream->pull_await(sock_data);
  if (r < 0) {
    LOG_ERROR << get_name(conn) << " pull failed: " << r;
    m->remove(id);
  }
  m->increase_recv_size(r);
  set_work_flag(WorkFlag::Command, true);
}
  
plain::net::detail::Task<>
Basic::Impl::process_output_await(Basic *conn) noexcept {
  auto m = manager.lock();
  if (!m) co_return;
  auto sock_data = m->get_sock_data();
  for (;;) {
    auto r = co_await ostream->push_await(sock_data);
    if (ostream->size() == 0) break;
    if (r < 0) {
      LOG_ERROR << get_name(conn) << " pull failed: " << r;
      m->remove(id);
      break;
    }
    m->increase_send_size(r);
  }
}

bool Basic::Impl::process_command(Basic *conn) noexcept {
  assert(conn);
  if (!has_work_flag(WorkFlag::Command)) return true;
  scoped_executor_t check_flag([this]{
    if (istream->size() == 0)
      set_work_flag(WorkFlag::Command, false);
  });
  if (istream->size() == 0) return true;
  stream::decode_func func{stream::decode};
  packet::limit_t packet_limit;
  auto m = manager.lock();
  if (m) packet_limit = m->setting_.packet_limit;
  if (codec.decode) {
    func = codec.decode;
  } else if (m && m->codec().decode) {
    func = m->codec().decode;
  }
  if (error_times >= kCantPeekMaxCount) return false;
  for (size_t i = 0; i < kOnceHandlePacketCount; ++i) {
    auto r = func(istream.get(), packet_limit);
    auto e = get_error(r);
    if (e) {
      if (e->is_code(ErrorCode::NetPacketCantFill)) {
        set_work_flag(WorkFlag::Command, false);
        ++error_times;
        return true;
      } else if (e->is_code(ErrorCode::NetPacketNeedRecv)) {
        set_work_flag(WorkFlag::Command, false);
        return true;
      }
      LOG_ERROR << get_name(conn) << " error: " << e->code();
      return false;
    }
    auto p = std::get_if<std::shared_ptr<packet::Basic>>(&r);
    if (!p) return false; // impossible.
    // Handle packet.
    if ((*p)->is_call_request() || (*p)->is_call_notify()) {
      if (!handle_rpc_request(conn, *p)) return false;
    } else if ((*p)->is_call_response()) {
      if (!handle_rpc_response(conn, *p)) return false;
    } else if (dispatcher) {
      if (!dispatcher(conn, *p)) return false;
    } else if (m && m->dispatcher()) {
      if (!m->dispatcher()(conn, *p)) return false;
    } else {
      LOG_WARN << get_name(conn) << " packet unhandled: " << (*p)->id();
    }
    if (istream->size() == 0) break;
  }
  error_times = 0;
  return true;
}

bool Basic::Impl::process_exception() noexcept {
  if (!has_work_flag(WorkFlag::Except)) return true;
  return true;
}

bool Basic::Impl::work(Basic *conn) noexcept {
  if (!process_exception()) return false;
  if (!process_input(conn)) return false;
  if (!process_output(conn)) return false;
  if (!process_command(conn)) return false;
  return true;
}
  
/**
 * How to enqueue work banlance?
 **/
void Basic::Impl::enqueue_work() noexcept {
  auto m = manager.lock();
  if (!m || !m->running()) return;
  const auto _working = working.exchange(true, std::memory_order_acq_rel);
  if (_working) return;
  /*
  std::cout << "enqueue_work: " << this << " fd: " << socket->id() << 
    " working: " << _working << "|" <<
    working.load(std::memory_order_relaxed) << std::endl;
  */
  auto executor = m->get_executor();
  if (!static_cast<bool>(executor)) return;
  if (executor->shutdown_requested()) return;
  executor->post([id = id, m = m] {
      auto conn = m->get_conn(id);
      if (!conn) return;
      auto r = conn->work();
      conn->impl_->working.store(false, std::memory_order_relaxed);
      if (!r) {
        m->remove(id);
      } else {
        if (!conn->idle()) {
          conn->impl_->enqueue_work();
        }
      }
  });
}

bool Basic::Impl::has_work_flag(WorkFlag flag) const noexcept {
  std::unique_lock<std::mutex> lock{mutex};
  return work_flags & (0x1 << std::to_underlying(flag));
}
  
void Basic::Impl::set_work_flag(WorkFlag flag, bool enable) noexcept {
  std::unique_lock<std::mutex> lock{mutex};
  if (enable)
    work_flags |= (0x1 << std::to_underlying(flag));
  else
    work_flags &= ~(0x1 << std::to_underlying(flag));
#if 0
  // wait input finsh.
  if (flag == WorkFlag::Input) {
    auto m = manager.lock();
    if (m && socket->valid()) {
      if (enable)
        m->sock_remove(socket->id());
      else{
        m->sock_add(socket->id(), id);
        m->send_ctrl_cmd("w");
      }
    }
  }
#endif
}
  
bool Basic::Impl::exchange_work_flag(WorkFlag flag, bool enable) noexcept {
  std::unique_lock<std::mutex> lock{mutex};
  auto r = static_cast<bool>(work_flags & (0x1 << std::to_underlying(flag)));
  if (r == enable) return r;
  if (enable)
    work_flags |= (0x1 << std::to_underlying(flag));
  else
    work_flags &= ~(0x1 << std::to_underlying(flag));
  return r;
}

std::string Basic::Impl::get_name(const Basic *conn) noexcept {
  if (!conn->valid()) return {};
  std::string r;
  auto m = conn->impl_->manager.lock();
  if (m && !m->setting_.name.empty()) {
    r = m->get_name(conn->id());
  }
  return r;
}

bool Basic::Impl::handle_rpc_request(
  Basic *conn, std::shared_ptr<packet::Basic> packet) {
  auto m = manager.lock();
  if (!m) return false;
  auto dispatcher = m->rpc_dispatcher();
  if (dispatcher) {
    uint32_t index{0};
    if (packet->is_call_request())
      (*packet) >> index;
    auto r = dispatcher->dispatch(packet);
    auto e = get_error(r);
    int32_t error = e ? e->code() : std::to_underlying(ErrorCode::None);
    if (packet->is_call_request()) {
      auto p = std::make_shared<packet::Basic>();
      p->set_id(packet::kRpcResponseId);
      p->set_writeable(true);
      p->set_call_response(true);
      (*p) << index;
      (*p) << error;
      if (!e) {
        auto temp = std::get_if<std::shared_ptr<rpc::Packer>>(&r);
        if (temp) (*p) << (*temp)->vector();
      }
      p->set_writeable(false);
      conn->send(p);
    } else if (e) { // Notify
      LOG_ERROR << "rpc request error: " << error;
    }
  } else {
    LOG_WARN << "rpc request unhandled, check you " <<
      m->setting_.name << " codec";
  }

  return true;
}
  
bool Basic::Impl::handle_rpc_response(
  Basic *conn, std::shared_ptr<packet::Basic> packet) {

  uint32_t index{0};
  int32_t error{std::to_underlying(ErrorCode::None)};
  (*packet) >> index;
  if (0 == index) return true; // No call(notify?).
  (*packet) >> error;
  auto it = callings.find(index);
  if (it == callings.end()) {
    LOG_WARN << "rpc response no calling: " << index << " error: " << error;
    return false;
  }
  auto &call = it->second;
  try {
    if (error != std::to_underlying(ErrorCode::None)) {
      throw std::logic_error("error: " + std::to_string(error));
    }
    auto data = reinterpret_cast<const uint8_t *>(
      packet->data().data() + packet->offset());
    auto size = packet->data().size() - packet->offset();
    std::get<1>(call).set_value(rpc::Unpacker(data, size));
  } catch(...) {
    std::get<1>(call).set_exception(std::current_exception());
  }
  return true;
}
  
bool Basic::Impl::write(const std::shared_ptr<packet::Basic> &packet) noexcept {
  stream::encode_func func{stream::encode};
  if (codec.encode) {
    func = codec.encode;
  } else if (auto m = manager.lock(); m && m->codec().encode) {
    func = m->codec().encode;
  }
  auto bytes = func(packet);
  if (bytes.empty()) return false;
  auto r = ostream->write(bytes) == bytes.size();
  return r;
}

Basic::Basic() : impl_{std::make_unique<Impl>()} {

}

Basic::Basic(std::shared_ptr<socket::Basic> socket) :
  impl_{std::make_unique<Impl>(socket)} {

}

Basic::Basic(Basic &&object) noexcept = default;

Basic::~Basic() = default;

Basic &Basic::operator=(Basic &&object) noexcept = default;

bool Basic::init() noexcept {
  if (!impl_->socket->close()) return false;
  impl_->working.store(false, std::memory_order_relaxed);
  impl_->work_flags = 0;
  impl_->istream->clear();
  impl_->ostream->clear();
  impl_->callings.clear();
  impl_->call_index = 0;
  auto connect_call_key = get_callable_key(this, "__connect");
  s_callables.erase(connect_call_key);
  auto disconnect_call_key = get_callable_key(this, "__disconnect");
  s_callables.erase(disconnect_call_key);
  // if (!impl_->socket->create()) return false;
  return true;
}

bool Basic::idle() const noexcept {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  return !impl_->socket->valid() || (impl_->work_flags == 0);
}
  
bool Basic::shutdown(int32_t how) noexcept {
  if (!impl_->socket->valid()) return true;
  return impl_->socket->shutdown(how);
}
  
bool Basic::close() noexcept {
  if (!impl_->socket->valid()) return true;
  return impl_->socket->close();
}
  
bool Basic::valid() const noexcept {
  return impl_->socket->valid();
}
  
bool Basic::work() noexcept {
  return impl_->work(this);
}

void Basic::enqueue_work(WorkFlag flag) noexcept {
  if (impl_->socket->awaitable() &&
      (flag == WorkFlag::Input || flag == WorkFlag::Output)) {
    if (flag == WorkFlag::Input)
      impl_->process_input_await(this);
    else if (flag == WorkFlag::Output)
      impl_->process_output_await(this);
  } else {
    if (impl_->exchange_work_flag(flag, true)) return;
    // impl_->enqueue_work();
    auto m = impl_->manager.lock();
    if (m) m->enqueue(impl_->id); // now to banlance work
  }
}

plain::net::connection::id_t Basic::id() const noexcept {
  return impl_->id;
}

std::string Basic::name() const noexcept {
  return impl_->get_name(this);
}
  
void Basic::set_name(std::string_view name) noexcept {
  auto m = impl_->manager.lock();
  if (!m) return;
  m->set_name(impl_->id, name);
}
  
void Basic::set_id(id_t id) noexcept {
  impl_->id = id;
}
  
std::shared_ptr<plain::net::socket::Basic> Basic::socket() const noexcept {
  return impl_->socket;
}
  
/*
plain::net::stream::Basic &Basic::istream() {
  return *(impl_->istream);
}
  
plain::net::stream::Basic &Basic::ostream() {
  return *(impl_->ostream);
}
*/

void Basic::set_codec(const stream::codec_t &codec) noexcept {
  impl_->codec = codec;
}
  
void Basic::set_connect_callback(callable_func func) noexcept {
  auto key = get_callable_key(this, "__connect");
  s_callables[key] = func;
}
  
void Basic::set_disconnect_callback(callable_func func) noexcept {
  auto key = get_callable_key(this, "__disconnect");
  s_callables[key] = func;
}
  
void Basic::set_manager(std::shared_ptr<Manager> manager) noexcept {
  impl_->manager = manager;
}
  
void Basic::set_dispatcher(packet::dispatch_func func) noexcept {
  impl_->dispatcher = func;
}

bool Basic::send(const std::shared_ptr<packet::Basic> &packet) noexcept {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  auto r = impl_->write(packet);
  lock.unlock();
  if (r) enqueue_work(WorkFlag::Output);
  return r;
}

void Basic::on_connect() noexcept {
  check_callable(this, "__connect");
}
  
void Basic::on_disconnect() noexcept {
  impl_->istream->clear();
  impl_->ostream->clear();
  impl_->working.store(false, std::memory_order_relaxed);
  check_callable(this, "__disconnect");
}

void Basic::set_keep_alive(bool flag) const noexcept {
  impl_->keep_alive.store(flag, std::memory_order_relaxed);
}
  
bool Basic::is_keep_alive() const noexcept {
  return impl_->keep_alive.load(std::memory_order_relaxed);
}

bool Basic::send_call(
  const std::shared_ptr<packet::Basic> &packet,
  uint32_t index, const std::string &func_name,
  std::shared_ptr<std::promise<rpc::Unpacker>> promise) {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  packet->set_call_request(true);
  auto r = impl_->write(packet);
  if (r && static_cast<bool>(promise)) {
    impl_->callings.emplace(
      index, std::make_pair(func_name, std::move(*promise)));
  }
  lock.unlock();
  if (r) enqueue_work(WorkFlag::Output);
  // std::cout << "send_call: " << name() << std::endl;
  return r;
}
  
uint32_t Basic::new_call_index() noexcept {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  return ++(impl_->call_index);
}

void Basic::set_call_timeout(const std::chrono::milliseconds &timeout) noexcept {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  impl_->timeout = timeout.count();
}

void Basic::clear_call_timeout() noexcept {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  impl_->timeout = std::nullopt;
}

std::optional<int64_t> Basic::get_call_timeout() const noexcept {
  std::unique_lock<std::mutex> lock{impl_->mutex};
  return impl_->timeout;
}
