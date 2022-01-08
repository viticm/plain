#include "pf/basic/type/variable.h"
#include "pf/basic/logger.h"
#include "pf/engine/kernel.h"
#include "pf/script/interface.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/dynamic.h"
#include "pf/net/packet/handshake.h"
#include "pf/net/packet/forward.h"
#include "pf/net/packet/register_connection_name.h"
#include "pf/net/packet/routing.h"
#include "pf/net/packet/routing_lost.h"
#include "pf/net/packet/routing_request.h"
#include "pf/net/packet/routing_response.h"
#include "pf/net/packet/factorymanager.h"

std::unique_ptr< pf_net::packet::FactoryManager >
  g_packetfactory_manager{nullptr};

template<> 
pf_net::packet::FactoryManager *pf_basic::Singleton<
  pf_net::packet::FactoryManager>::singleton_ = nullptr;

namespace pf_net {

namespace packet {

FactoryManager *FactoryManager::getsingleton_pointer() {
  return singleton_;
}

FactoryManager &FactoryManager::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

FactoryManager::FactoryManager() :
  factories_{nullptr},
  size_{0xfffe},
  factory_size_{0},
  ready_{false},
  function_register_factories_{nullptr},
  function_is_valid_packet_id_{nullptr},
  function_is_valid_dynamic_packet_id_{nullptr},
  //function_is_encrypt_packet_id_{nullptr},
  function_packet_execute_{nullptr} {
  alloc_packets_.init(NET_PACKET_FACTORYMANAGER_ALLOCMAX);
}

FactoryManager::~FactoryManager() {
  Assert(factories_ != nullptr);
  uint16_t i;
  pf_basic::hashmap::Template<int64_t, Interface *>::iterator_t _iterator;
  for (_iterator = alloc_packets_.begin(); 
       _iterator != alloc_packets_.end(); 
       ++_iterator) {
    safe_delete(_iterator->second);
  }
  for (i = 0; i < size_; ++i) {
    safe_delete(factories_[i]);
  }
  safe_delete_array(factories_);
  safe_delete_array(packet_alloc_size_);
}

bool FactoryManager::init() {
  if (ready()) return true;
  Assert(size_ > 0);
  factories_ = new Factory * [size_];
  Assert(factories_);
  packet_alloc_size_ = new uint32_t[size_];
  Assert(packet_alloc_size_);
  id_indexs_.init(size_); //ID索引数组初始化
  uint16_t i;
  //memset(factories_, 0, sizeof(Factory) * size_);
  //memset(packet_alloc_size_, 0, sizeof(uint32_t) * size_);
  for (i = 0; i < size_; ++i) {
    factories_[i] = nullptr;
    packet_alloc_size_[i] = 0;
  }

  if (!is_null(function_register_factories_) && 
      !(*function_register_factories_)()) return false;
  //Framework owner.
  add_factory(new HandshakeFactory);
  add_factory(new RegisterConnectionNameFactory);
  add_factory(new RoutingFactory);
  add_factory(new RoutingLostFactory);
  add_factory(new RoutingRequestFactory);
  add_factory(new RoutingResponseFactory);
  add_factory(new ForwardFactory);
  ready_ = true;
  return true;
}

Interface *FactoryManager::packet_create(uint16_t packet_id, bool dynamic) {
  Interface *packet = nullptr;
  std::unique_lock<std::mutex> autolock(mutex_);
  if (is_valid_dynamic_packet_id(packet_id) || dynamic) {
    packet = new Dynamic(packet_id);
  } else {
    bool is_find = id_indexs_.isfind(packet_id);
    uint16_t index = id_indexs_.get(packet_id);
    if (!is_find || nullptr == factories_[index]) {
      Assert(false);
      return nullptr;
    }
    packet = factories_[index]->packet_create();
    ++(packet_alloc_size_[index]);
  }
  if (packet) { //Memory safe.
    int64_t pointer = POINTER_TOINT64(packet);
    alloc_packets_.add(pointer, packet);
  }
  return packet;
}

uint32_t FactoryManager::packet_max_size(uint16_t packet_id) {
  uint32_t result = 0;
  std::unique_lock<std::mutex> autolock(mutex_);
  bool find_it = id_indexs_.isfind(packet_id);
  uint16_t index = id_indexs_.get(packet_id);
  if (!find_it || nullptr == factories_[index]) {
    char temp[FILENAME_MAX] = {0};
    snprintf(temp, 
             sizeof(temp) - 1, 
             "packetid: %d not register in factory!", 
             packet_id);
    AssertEx(false, temp);
    return result;
  }
  result = factories_[index]->packet_max_size();
  return result;
}

void FactoryManager::packet_remove(Interface *packet) {
  std::unique_lock<std::mutex> autolock(mutex_);
  if (nullptr == packet) {
    Assert(false);
    return;
  }
  uint16_t packet_id = packet->get_id();
  if (instanceof(packet, packet::Dynamic)) {
      int64_t pointer = POINTER_TOINT64(packet);
      alloc_packets_.remove(pointer);
      safe_delete(packet);
  } else {
    bool is_find = id_indexs_.isfind(packet_id);
    uint16_t index = id_indexs_.get(packet_id);
    int64_t pointer = POINTER_TOINT64(packet);
    alloc_packets_.remove(pointer);
    safe_delete(packet);
    if (!is_find) {
      SLOW_ERRORLOG(
          NET_MODULENAME, 
          "[net.packet] (FactoryManager::packet_remove) error,"
          " can't find id index for packeid: %d",
          packet_id);
    } else {
      --(packet_alloc_size_[index]);
    }
  }
}

void FactoryManager::add_factory(Factory *factory) {
  if (is_null(factory)) return;
  bool is_find = id_indexs_.isfind(factory->packet_id());
  uint16_t index = 
    is_find ? id_indexs_.get(factory->packet_id()) : factory_size_;
  if (factories_[index] != nullptr) {
    Assert(false);
    return;
  }
  if (!is_find) {
    ++factory_size_;
    id_indexs_.add(factory->packet_id(), index);
    factories_[index] = factory;
  } else {
    SLOW_WARNINGLOG(NET_MODULENAME, 
                    "[net.packet] (FactoryManager::add_factory) repeat add"
                    " packet id: %d",
                    factory->packet_id());
  }
}

bool FactoryManager::is_valid_packet_id(uint16_t id) {
  bool result = false;
  if (!function_is_valid_packet_id_)
    return NET_PACKET_ID_NORMAL_BEGIN <= id && id <= NET_PACKET_ID_NORMAL_END;
  result = (*function_is_valid_packet_id_)(id);
  return result;
}

bool FactoryManager::is_encrypt_packet_id(uint16_t id) {
  return id >= NET_PACKET_HANDSHAKE && id < 0xffff;
}

bool FactoryManager::is_valid_dynamic_packet_id(uint16_t id) {
  bool result = false;
  if (!function_is_valid_dynamic_packet_id_) 
    return NET_PACKET_ID_DYNAMIC_BEGIN <= id && id <= NET_PACKET_ID_DYNAMIC_END;
  result = (*function_is_valid_dynamic_packet_id_)(id);
  return result;
}

uint32_t FactoryManager::packet_execute(
  connection::Basic *connection, 
  Interface *packet, 
  const std::string &original) {
  if (!function_packet_execute_) {
    auto script = ENGINE_POINTER->get_script();
    if (is_null(script)) {
      FAST_WARNINGLOG(NET_MODULENAME, 
                      "[net.packet] (FactoryManager::packet_execute) id: %d"
                      " will not handle", 
                      packet->get_id());
      return kPacketExecuteStatusContinue;
    }
    if (!is_valid_dynamic_packet_id(packet->get_id()) && 
        GLOBALS["default.script.netpack"] == 0) {
      return kPacketExecuteStatusError;
    } else {
      dynamic_cast<Dynamic *>(packet)->set_readable(true); //Set enable read.
      std::string funcname = ENGINE_POINTER->get_script_function(connection);
      if (funcname != "") {
        pf_basic::type::variable_array_t params;
        pf_basic::type::variable_array_t r;
        params.emplace_back(POINTER_TOINT64(packet));
        auto name = connection->name();
        if (name != "") {
          params.emplace_back(name);
        } else {
          params.emplace_back(connection->get_id());
        }
        if (original != "") params.emplace_back(original);
        script->call(funcname, params, r);
      }
      return kPacketExecuteStatusContinue;
    }
  }
  //std::cout << "packet_execute 1" << std::endl;
  uint32_t result = (*function_packet_execute_)(connection, packet);
  return result;
}

} //namespace packet

} //namespace pf_net
