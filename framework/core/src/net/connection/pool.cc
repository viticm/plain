#include "pf/basic/io.tcc"
#include "pf/sys/assert.h"
#include "pf/net/connection/manager/basic.h"
#include "pf/net/connection/pool.h"

namespace pf_net {

namespace connection {

Pool::Pool() : 
  ready_{false},
  position_{0},
  size_{0},
  max_size_{NET_CONNECTION_POOL_SIZE_DEFAULT} {
  connections_.clear();
}

Pool::~Pool() {
  //do nothing.
}

bool Pool::init(uint32_t max_size) {
  if (ready_) return true;
  max_size_ = max_size;
  for (uint32_t i = 0; i < max_size_; ++i)
    connections_.push_back(nullptr);
  position_ = 0;
  size_ = max_size_;
  ready_ = true;
  return true;
}

bool Pool::init_data(uint16_t index, Basic *connection) {
  Assert(connection);
  Assert(index >= 0 && index < max_size_);
  std::unique_ptr< Basic > ptr(connection);
  connections_[index] = std::move(ptr);
  connections_[index]->set_id(index);
  connections_[index]->set_empty(true);
  return true;
}

Basic *Pool::get(int16_t id) {
  Basic *connection = nullptr;
  if (static_cast<uint32_t>(id) > max_size_) return connection;
  connection = connections_[id].get();
  if (nullptr == connection) pf_basic::io_cerr("Pool::get is nullptr");
  return connection;
}

Basic *Pool::create(bool clear) {
  if (0 == size_) return nullptr;  //Can used connection count.
  Basic *connection = nullptr;
  std::unique_lock<std::mutex> autolock(mutex_);
  /* Closure to get connection. */
  auto func = [this, clear, &connection]() {
    if (connections_[position_] && connections_[position_]->empty()) {
      connection = connections_[position_].get();
      if (clear) connection->clear();
      connection->set_empty(false);
      ++position_; //Position to next.
    }
  };
  auto lastpos = position_;
  //Right.
  for (; position_ < max_size_; ++position_) {
    func();
    if (connection) {
      --size_;
      break;
    }
  }
  //Left.
  if (is_null(connection)) {
    for (position_ = 0; position_ < lastpos; ++position_) {
      func();
      if (connection) {
        --size_;
        break;
      }
    }
  }
  return connection;
}

void Pool::remove(int16_t id) {
  if (static_cast<uint32_t>(id) > max_size_) {
    Assert(false);
    return;
  }
  if (!is_null(connections_[id])) connections_[id]->clear(); //清除连接信息
  ++size_;
}

bool Pool::create_default_connections() {
  if (!is_null(connections_[0]) && !is_null(connections_[max_size_ - 1])) 
    return true;
  std::unique_lock<std::mutex> autolock(mutex_);
  for (uint16_t i = 0; i < max_size_; i++) {
    auto connection = new connection::Basic();
    if (is_null(connection)) return false;
    connection->set_protocol(manager::Basic::protocol_default());
    init_data(i, connection);
  }
  return true;
}

} //namespace connection

} //namespace pf_net
