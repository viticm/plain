#include "plain/net/connection/basic.h"
#include <map>
#include "plain/basic/logger.h"
#include "plain/concurrency/executor/basic.h"
#include "plain/net/socket/basic.h"
#include "plain/net/stream/basic.h"
#include "plain/net/connection/manager.h"
#include "plain/net/packet/basic.h"

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
  mutable std::mutex mutex;

  bool process_input(Basic *conn) noexcept;
  bool process_output(Basic *conn) noexcept;
  bool process_command(Basic *conn) noexcept;
  bool process_exception() noexcept;
  bool work(Basic *conn) noexcept;
  void enqueue_work() noexcept; 
  bool has_work_flag(WorkFlag flag) const noexcept;
  void set_work_flag(WorkFlag flag, bool enable) noexcept;
  static std::string get_name(const Basic *conn) noexcept;
};

Basic::Impl::Impl() :
  id{kInvalidId},
  socket{std::make_shared<socket::Basic>()},
  istream{std::make_unique<stream::Basic>(socket)},
  ostream{std::make_unique<stream::Basic>(socket)},
  codec{}, manager{} {

}

Basic::Impl::~Impl() = default;

bool Basic::Impl::process_input(Basic *conn) noexcept {
  if (!has_work_flag(WorkFlag::Input)) return true;
  assert(conn);
  if (!socket->valid()) return false;
  auto r = istream->pull();
  if (r < 0) {
    LOG_ERROR << get_name(conn) << " pull failed: " << r;
    return false;
  }
  // std::cout << "process_input: " << conn->name() << "|" << r << std::endl;
  if (socket->avail() == 0)
    set_work_flag(WorkFlag::Input, false);
  return true;
}

bool Basic::Impl::process_output(Basic *conn) noexcept {
  if (!has_work_flag(WorkFlag::Output)) return true;
  assert(conn);
  if (!socket->valid()) return false;
  {
    std::unique_lock<std::mutex> lock{mutex};
    auto r = ostream->push();
    // std::cout << "process_output: " << ostream->size() << std::endl;
    if (r < 0) {
      LOG_ERROR << get_name(conn) << " push failed: " << r;
      return false;
    }
  }
  if (ostream->size() == 0)
    set_work_flag(WorkFlag::Output, false);
  return true;
}

bool Basic::Impl::process_command(Basic *conn) noexcept {
  assert(conn);
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
  bool clear_error{true};
  for (size_t i = 0; i < kOnceHandlePacketCount; ++i) {
    auto r = func(istream.get(), packet_limit);
    auto e = get_error(r);
    if (e) {
      if (e->is_code(ErrorCode::NetPacketCantFill)) {
        ++error_times;
        clear_error = false;
        return true;
      } else if (e->is_code(ErrorCode::NetPacketNeedRecv)) {
        return true;
      }
      LOG_ERROR << get_name(conn) << " error: " << e->code();
      return false;
    }
    auto p = std::get_if<std::shared_ptr<packet::Basic>>(&r);
    if (!p) return false; // impossible.
    // Handle packet.
    if (dispatcher) {
      if (!dispatcher(conn, *p)) return false;
    } else if (auto m = manager.lock(); m && m->dispatcher()) {
      if (!m->dispatcher()(conn, *p)) return false;
    } else {
      LOG_WARN << get_name(conn) << " packet unhandled: " << (*p)->id();
    }
  }
  if (clear_error) error_times = 0;
  if (istream->size() == 0)
    set_work_flag(WorkFlag::Command, false);
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
  
void Basic::Impl::enqueue_work() noexcept {
  auto m = manager.lock();
  if (!m || !m->running()) return;
  const auto _working = working.exchange(true, std::memory_order_relaxed);
  if (_working) return;
  // std::cout << "enqueue_work: " << this << " m: " << m << std::endl;
  if (m->get_executor().shutdown_requested()) return;
  m->get_executor().post([id = id, m = m] {
      auto conn = m->get_conn(id);
      if (!conn) return;
      auto r = conn->work();
      conn->impl_->working.store(false, std::memory_order_relaxed);
      // std::cout << "work end: " << conn->impl_ << std::endl;
      if (!r) {
        m->remove(id);
      } else {
        if (!conn->idle()) {
          m->execute([conn]{ conn->impl_->enqueue_work(); });
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
}

std::string Basic::Impl::get_name(const Basic *conn) noexcept {
  if (!conn->valid()) return {};
  std::string r;
  auto m = conn->impl_->manager.lock();
  if (m && !m->setting_.name.empty()) {
    r += m->setting_.name;
  } else {
    r += "unknown";
  }
  r += ".";
  r += std::to_string(conn->id());
  return r;
}

Basic::Basic() : impl_{std::make_unique<Impl>()} {

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
  auto connect_call_key = get_callable_key(this, "__connect");
  s_callables.erase(connect_call_key);
  auto disconnect_call_key = get_callable_key(this, "__disconnect");
  s_callables.erase(disconnect_call_key);
  // if (!impl_->socket->create()) return false;
  return true;
}

bool Basic::idle() const noexcept {
  return !impl_->socket->valid() ||
    (impl_->socket->avail() == 0 && impl_->work_flags == 0);
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
  if (impl_->has_work_flag(flag)) return;
  impl_->set_work_flag(flag, true);
  impl_->enqueue_work();
}

plain::net::connection::id_t Basic::id() const noexcept {
  return impl_->id;
}

std::string Basic::name() const noexcept {
  return impl_->get_name(this);
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
  stream::encode_func func{stream::encode};
  if (impl_->codec.encode) {
    func = impl_->codec.encode;
  } else if (
    auto manager = impl_->manager.lock(); manager && manager->codec().encode) {
    func = manager->codec().encode;
  }
  auto bytes = func(packet);
  if (bytes.empty()) return false;
  auto r = impl_->ostream->write(bytes);
  lock.unlock();
  if (r) enqueue_work(WorkFlag::Output);
  return r;
}

void Basic::on_connect() noexcept {
  check_callable(this, "__connect");
}
  
void Basic::on_disconnect() noexcept {
  check_callable(this, "__disconnect");
}
