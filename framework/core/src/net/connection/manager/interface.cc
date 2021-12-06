#include "pf/basic/logger.h"
#include "pf/sys/thread.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/connection/manager/interface.h"

namespace pf_net {

namespace connection {

namespace manager {

Interface::Interface() :
  connection_max_size_{NET_CONNECTION_MAX},
  fdsize_{0},
  ready_{false}, 
  connection_idset_{nullptr},
  max_size_{0},
  size_{0},
  send_bytes_{0},
  receive_bytes_{0},
  onestep_accept_{NET_ONESTEP_ACCEPT_DEFAULT},
  pool_{nullptr},
  callback_disconnect_{nullptr},
  callback_connect_{nullptr} {
}

Interface::~Interface() {
  safe_delete_array(connection_idset_);
}

bool Interface::init(uint16_t maxcount) {
  if ((int16_t)maxcount == -1) return false;
  if (is_ready()) return true; //有内存分配的请参考此方式避免再次分配内存
  size_ = 0;
  max_size_ = maxcount;
  connection_idset_ = new int16_t[max_size_];
  Assert(connection_idset_);
  if (is_null(connection_idset_)) return false;
  memset(connection_idset_, ID_INVALID, sizeof(int16_t) * max_size_);
  auto pool = new connection::Pool();
  if (is_null(pool)) return false;
  std::unique_ptr<connection::Pool> pointer{pool};
  pool_ = std::move(pointer);
  if (!pool_->init(maxcount)) return false;
  //if (!poll_set_max_size(maxcount)) return false;
  thread_id_ = std::this_thread::get_id();
  if (is_null(NET_PACKET_FACTORYMANAGER_POINTER)) {
    using FactoryManager = pf_net::packet::FactoryManager;
    std::unique_ptr<FactoryManager> tmp(new FactoryManager());
    g_packetfactory_manager = std::move(tmp);
  }
  if (is_null(NET_PACKET_FACTORYMANAGER_POINTER)) return false;
  if (!NET_PACKET_FACTORYMANAGER_POINTER->init()) return false;
  if (!pool_init(maxcount)) return false;
  ready_ = true;
  return true;
}

bool Interface::pool_init(uint16_t connectionmax) {
  if (is_null(pool_)) return false;
  connection_max_size_ = connectionmax;
  if (!pool_->init(connection_max_size_)) return false;
  if (!pool_->create_default_connections(this)) return false;
  return true;
}

void Interface::pool_set(connection::Pool *pool) {
  if (pool_) {
    SLOW_WARNINGLOG(NET_MODULENAME,
                    "[connection.manager] (Interface::init_pool)"
                    " the class pool is init, now will delete it first");
  }
  std::unique_ptr<connection::Pool> pointer{pool};
  pool_ = std::move(pointer);
}

bool Interface::heartbeat(uint32_t) {
  bool result = true;
  return result;
}

bool Interface::add(connection::Basic *connection) {
  Assert(connection);
  if (size_ >= max_size_) return false;
  //首先处理socket
  if (!socket_add(connection->socket()->get_id(), connection->get_id()))
    return false;
  //再处理管理器ID
  if (ID_INVALID == connection_idset_[size_]) {
    connection_idset_[size_] = connection->get_id();
    connection->set_managerid(size_);
    ++size_;
    Assert(size_ <= max_size_);
  } else {
    Assert(false);
  }
  connection->set_disconnect(false); //connect is success
  connection->set_empty(false);      //Pool use flag.
  on_connect(connection);
  if (!is_null(callback_connect_)) callback_connect_(connection);
  return true;
}

bool Interface::remove(int16_t id) {
  Assert(size_ > 0);
  connection::Basic *connection = nullptr;
  connection = pool_->get(id);
  if (nullptr == connection) {
    Assert(false);
    return false;
  }
  int16_t managerid = connection->get_managerid();
  if (managerid >= static_cast<int16_t>(size_)) {
    Assert(false);
    return false;
  }
  //Swap last.
  --size_;
  connection_idset_[managerid] = ID_INVALID;
  if (size_ != managerid) {
    auto lastid = connection_idset_[size_];
    connection = pool_->get(lastid);
    connection_idset_[managerid] = lastid;
    connection_idset_[size_] = ID_INVALID;
    connection->set_managerid(managerid);
  }
  return true;
}

bool Interface::erase(connection::Basic *connection) {
  if (is_null(connection)) return false;
  //First remove socket.
  socket_remove(connection->socket()->get_id());
  //Second clean in connection manager.
  if (!remove(connection->get_id())) return false;
#ifdef _DEBUG
  FAST_LOG(NET_MODULENAME, 
           "[net.connection.manager] (Interface::erase) id: %d", 
           connection->get_id());
#endif
  return true;
}

bool Interface::remove(connection::Basic *connection) {
  on_disconnect(connection);
  if (!is_null(callback_disconnect_)) callback_disconnect_(connection);
  if (connection->name() != "") 
    connection_names_[connection->name()] = -1;
  if (!erase(connection)) return false; 
  connection->disconnect();
  pool_->remove(connection->get_id());
  return true;
}

bool Interface::destroy() {
  uint16_t i = 0;
  for (i = 0; i < size_; ++i) {
    if (ID_INVALID == connection_idset_[i]) {
      SLOW_ERRORLOG(NET_MODULENAME, 
                    "[net.connection.manager] (Interface::destroy) error!"
                    " ID_INVALID == connection_idset_[%d]",
                    i);
      continue;
    }
    connection::Basic *connection = get(connection_idset_[i]);
    if (nullptr == connection) {
      SLOW_ERRORLOG(NET_MODULENAME,
                    "[net.connection.manager](Interface::destroy) error!"
                    " connection is nullptr, id: %d",
                    connection_idset_[i]);
      continue;
    }
    remove(connection);
  }
  return false;
}

connection::Basic *Interface::get(int16_t id) {
  if (id < 0 || id > max_size_) return nullptr;
  connection::Basic *connection = nullptr;
  connection = pool_->get(id);
  Assert(connection);
  return connection;
}

int16_t* Interface::get_idset() {
  return connection_idset_;
}

bool Interface::hash() {
  bool result = connection_idset_[0] != ID_INVALID;
  return result;
}

int32_t Interface::get_onestep_accept() const {
  return onestep_accept_;
}

void Interface::set_onestep_accept(int32_t count) {
  onestep_accept_ = count;
}

uint64_t Interface::get_send_bytes() {
  uint64_t result = send_bytes_;
  send_bytes_ = 0;
  return result;
}
   
uint64_t Interface::get_receive_bytes() {
  uint64_t result = receive_bytes_;
  receive_bytes_ = 0;
  return result;
}

connection::Pool *Interface::get_pool() {
  if (is_null(pool_)) {
    std::unique_ptr<connection::Pool> pointer{new connection::Pool()};
    pool_ = std::move(pointer);
  }
  Assert(pool_);
  return pool_.get();
}

connection::Basic *Interface::get(uint16_t id) {
  connection::Basic *connection = nullptr;
  if ((int16_t)id >= 0 && id < max_size_) {
    connection = pool_->get(id);
    Assert(connection);
  }
  return connection;
}

bool Interface::send(packet::Interface *packet, 
                     uint16_t connectionid, 
                     uint32_t flag) {
  std::unique_lock<std::mutex> autolock(mutex_);
  if (cache_.queue[cache_.tail].packet) {
    bool result = cache_resize();
    Assert(result);
  }
  cache_.queue[cache_.tail].packet = packet;
  cache_.queue[cache_.tail].connectionid = connectionid;
  cache_.queue[cache_.tail].flag = flag;
  ++cache_.tail;
  if (cache_.tail > cache_.size) cache_.tail = 0;
  return true;
}
   
bool Interface::process_command_cache() {
  bool result = false;
  if (!NET_PACKET_FACTORYMANAGER_POINTER) return result;
  uint32_t _result = kPacketExecuteStatusContinue;
  for (uint32_t i = 0; i < cache_.size; ++i) {
    packet::Interface *packet = nullptr;
    uint16_t connectionid = static_cast<uint16_t>(ID_INVALID);
    uint32_t flag = kPacketFlagNone;
    bool needremove = true;
    result = recv(packet, connectionid, flag);
    if (!result) break;
    if (is_null(packet)) {
      SaveErrorLog();
      break;
    }
    
    if (kPacketFlagRemove == flag) {
      NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
      break;
    }
    
    int32_t _connectionid = static_cast<int32_t>(connectionid);
    if (ID_INVALID == _connectionid || ID_INVALID_EX == _connectionid) {
      try {
        packet->execute(nullptr);
      } catch (...) {
        SaveErrorLog();
        _result = kPacketExecuteStatusError;
      }
      switch (_result) {
        case kPacketExecuteStatusError:
          break;
        case kPacketExecuteStatusBreak:
          break;
        case kPacketExecuteStatusContinue:
          break;
        case kPacketExecuteStatusNotRemove:
          needremove = false;
          break;
        case kPacketExecuteStatusNotRemoveError:
          needremove = false;
          break;
        default:
          break;
      }
    } else {
      connection::Basic *connection = get(connectionid);
      if (connection) {
        try {
          packet->execute(connection);
        } catch (...) {
          SaveErrorLog();
          _result = kPacketExecuteStatusError;
        }
        switch (_result) {
          case kPacketExecuteStatusError:
            break;
          case kPacketExecuteStatusBreak:
            break;
          case kPacketExecuteStatusContinue:
            break;
          case kPacketExecuteStatusNotRemove:
            needremove = false;
            break;
          case kPacketExecuteStatusNotRemoveError:
            needremove = false;
            remove(connection);
            break;
          default:
            break;
        }
      } else {
        SLOW_ERRORLOG(NET_MODULENAME,
                      "[net.connection.manager] (Interface::process_command_cache)"
                      " the connection is nullptr id: %d, packet id: %d",
                      connectionid,
                      packet->get_id());
        Assert(false);
      }
    }
    if (needremove) NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
  }
  return true;
}

bool Interface::recv(packet::Interface *&packet, 
                     uint16_t &connectionid, 
                     uint32_t &flag) {
  std::unique_lock<std::mutex> autolock(mutex_);
  if (is_null(cache_.queue[cache_.head].packet)) return false;
  packet = cache_.queue[cache_.head].packet;
  connectionid = cache_.queue[cache_.head].connectionid;
  flag = cache_.queue[cache_.head].flag;
  cache_.queue[cache_.head].packet = nullptr;
  cache_.queue[cache_.head].connectionid = static_cast<uint16_t>(ID_INVALID);
  cache_.queue[cache_.head].flag = kPacketFlagNone;
  ++cache_.head;
  if (cache_.head > cache_.size) cache_.head = 0;
  return true;
}
   
bool Interface::cache_resize() {
  packet::queue_t *queuenew = 
    new packet::queue_t[cache_.size + NET_MANAGER_CACHE_SIZE];
  if (is_null(queuenew)) return false;
  if (cache_.head < cache_.tail) {
    memcpy((char *)queuenew, 
           (char *)&(cache_.queue[cache_.head]), 
           sizeof(packet::queue_t) * (cache_.tail - cache_.head));
  } else {
    memcpy((char *)queuenew, 
           (char *)&(cache_.queue[cache_.head]), 
           sizeof(packet::queue_t) * (cache_.size - cache_.head));
    memcpy((char *)&queuenew[cache_.size - cache_.head], 
           (char *)cache_.queue, 
           sizeof(packet::queue_t) * cache_.tail);
  }
  memset((char *)cache_.queue, 0, sizeof(packet::queue_t) * cache_.size);
  safe_delete_array(cache_.queue);
  cache_.queue = queuenew;
  cache_.head = 0;
  cache_.tail = cache_.size;
  cache_.size = cache_.size + NET_MANAGER_CACHE_SIZE;
  FAST_DEBUGLOG(NET_MODULENAME,
                "[net.connection.manager] (Interface::cache_resize)"
                " from: %d, to: %d",
                cache_.size - NET_MANAGER_CACHE_SIZE,
                cache_.size);
  return true;
}

void Interface::broadcast(packet::Interface *packet) {
  for (int32_t i = 0; i < size_; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    auto connection = get(connection_idset_[i]);
    if (connection) connection->send(packet);
  }
}

bool Interface::checkpool(bool log) {
  if (is_null(pool_) && log) {
    SLOW_ERRORLOG(NET_MODULENAME,
                  "[%s] connection::manager::Interface::checkpool"
                  " error: pool null",
                  NET_MODULENAME);
    return false;
  }
  return is_null(pool_) ? false : true;
}

} //namespace manager

} //namespace connection

} //namespace pf_net
