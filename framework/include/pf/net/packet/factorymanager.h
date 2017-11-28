/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id factorymanager.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/22 14:16
 * @uses net packet factory manager
 *       cn: 该类在2.0正式版需要重构。
 */
#ifndef PF_NET_PACKET_FACTORYMANAGER_H_
#define PF_NET_PACKET_FACTORYMANAGER_H_

#include "pf/sys/thread.h"
#include "pf/basic/singleton.tcc"
#include "pf/basic/hashmap/template.h"
#include "pf/net/config.h"
#include "pf/net/packet/factory.h"
#include "pf/net/connection/basic.h"

namespace pf_net {

namespace packet {

//目前指针函数有些多，是否考虑更好的方式？ viticm
typedef bool (__stdcall *function_register_factories)();
typedef bool (__stdcall *function_is_valid_packet_id)(uint16_t id);
typedef bool (__stdcall *function_is_valid_dynamic_packet_id)(uint16_t id);
typedef bool (__stdcall *function_is_encrypt_packet_id)(uint16_t id);
typedef uint32_t (__stdcall *function_packet_execute)(connection::Basic *, Interface *);

class PF_API FactoryManager : public pf_basic::Singleton<FactoryManager> {

 public:
   FactoryManager();
   ~FactoryManager();

 public:
   uint32_t *packet_alloc_size_;

 public:
   static FactoryManager &getsingleton();
   static FactoryManager *getsingleton_pointer();
 
 public:
   bool init();
   //根据消息类型从内存里分配消息实体数据（允许多线程同时调用，必须用removepacket释放）
   Interface *packet_create(uint16_t packetid);
   //根据消息类型取得对应消息的最大尺寸（允许多线程同时调用）
   uint32_t packet_max_size(uint16_t packetid);
   //删除消息实体（允许多线程同时调用，必须和createpacket成对出现）
   void packet_remove(Interface *packet);
   bool is_valid_packet_id(uint16_t id); //packetid is valid
   bool is_valid_dynamic_packet_id(uint16_t id); //dynamic packet id is valid
   void set_size(uint16_t size) { size_ = size; };
   uint16_t size() const { return size_; };
   void add_factory(Factory *factory);
   bool is_encrypt_packet_id(uint16_t id); //packetid is encrypt id
   uint32_t packet_execute(connection::Basic *connection, Interface *packet);

 public: //exports
   void set_function_register_factories(function_register_factories function) {
     function_register_factories_ = function;
   };
   void set_function_is_valid_packet_id(function_is_valid_packet_id function) {
     function_is_valid_packet_id_ = function;
   }
   void set_function_is_encrypt_packet_id(
       function_is_valid_packet_id function) {
     function_is_valid_packet_id_ = function;
   }
   void set_function_is_valid_dynamic_packet_id(
       function_is_valid_dynamic_packet_id function) {
     function_is_valid_dynamic_packet_id_ = function;
   }
   void set_function_packet_execute(function_packet_execute function) {
     function_packet_execute_ = function;
   }
   bool ready() const { return ready_; };

 private:
   Factory **factories_;
   pf_basic::hashmap::Template<uint16_t, uint16_t> id_indexs_;
   pf_basic::hashmap::Template<int64_t, Interface *> alloc_packets_;
   uint16_t size_;
   uint16_t factory_size_;
   std::mutex mutex_;
   bool ready_; //凡是有内存的初始化都需加上这个标记，以检测再次初始化的情况
   
 private: //exports
   function_register_factories function_register_factories_;
   function_is_valid_packet_id function_is_valid_packet_id_;
   function_is_valid_dynamic_packet_id function_is_valid_dynamic_packet_id_;
   function_is_encrypt_packet_id function_is_encrypt_packet_id_;
   function_packet_execute function_packet_execute_;

};

}; //namespace packet

}; //namespace pap_common_net

PF_API extern std::unique_ptr< pf_net::packet::FactoryManager > 
  g_packetfactory_manager;

#define NET_PACKET_FACTORYMANAGER_POINTER \
  pf_net::packet::FactoryManager::getsingleton_pointer()

#endif //PF_NET_PACKETFACTORY_H_
