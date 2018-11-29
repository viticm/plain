#include "pf/basic/logger.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/dynamic.h"
#include "pf/net/packet/handshake.h"
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
  size_{1},
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
  /**
  if (!function_is_valid_packet_id_ || !function_register_factories_) {
    SLOW_ERRORLOG(
        NET_MODULENAME, 
        "[net.packet] (FactoryManager::init) error,"
        " the register factories and is valid packet id"
        " function pointer is nullptr");
    return false;
  }
  **/
  factories_ = new Factory * [size_];
  Assert(factories_);
  packet_alloc_size_ = new uint32_t[size_];
  Assert(packet_alloc_size_);
  id_indexs_.init(size_); //ID索引数组初始化
  uint16_t i;
  for (i = 0; i < size_; ++i) {
    factories_[i] = nullptr;
    packet_alloc_size_[i] = 0;
  }
  if (!is_null(function_register_factories_) && 
      !(*function_register_factories_)()) return false;
  //Handshake.
  add_factory(new HandshakeFactory);
  ready_ = true;
  return true;
}

Interface *FactoryManager::packet_create(uint16_t packet_id) {
  Interface *packet = nullptr;
  std::unique_lock<std::mutex> autolock(mutex_);
  if (is_valid_dynamic_packet_id(packet_id)) {
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
  if (is_valid_dynamic_packet_id(packet_id)) {
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
  connection::Basic *connection, Interface *packet) {
  std::cout << "packet_execute 0" << std::endl;
  if (!function_packet_execute_) return kPacketExecuteStatusError;
  std::cout << "packet_execute 1" << std::endl;
  uint32_t result = (*function_packet_execute_)(connection, packet);
  return result;
}

} //namespace packet

} //namespace pf_net
