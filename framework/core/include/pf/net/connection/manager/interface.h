/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id interface.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/08/09 20:22
 * @uses connection manager class
 *       With ten million with rand and mix connections pass in 2017-8-9.
 *       With more the 25t buffers send and recv in single connection in 2017-8-26(three days).
 */
#ifndef PF_NET_CONNECTION_MANAGER_BASE_H_
#define PF_NET_CONNECTION_MANAGER_BASE_H_

#include "pf/net/connection/manager/config.h"
#include "pf/sys/thread.h"
#include "pf/net/packet/interface.h"
#include "pf/net/protocol/basic.h"
#include "pf/net/connection/basic.h"
#include "pf/net/connection/pool.h"

namespace pf_net {

namespace connection {

namespace manager {

class PF_API Interface {

 public:
   Interface();
   virtual ~Interface();
 
 public:
   bool init(uint16_t maxcount = NET_CONNECTION_MAX);
   bool pool_init(uint16_t connectionmax = NET_CONNECTION_MAX);
   void pool_set(connection::Pool *pool);
   bool add(connection::Basic *connection);

 public:
   virtual bool heartbeat(uint32_t time = 0);
   //从管理器中移除连接
   virtual bool remove(int16_t id);
   //删除连接包括管理器、socket
   virtual bool erase(connection::Basic *connection);
   //彻底删除连接，管理器、socket、pool
   virtual bool remove(connection::Basic *connection);
   //清除管理器中所有连接
   virtual bool destroy();
   connection::Basic *get(int16_t id);
   virtual bool socket_add(int32_t socketid, int16_t connectionid) = 0;
   virtual bool socket_remove(int32_t socketid) = 0;
   virtual bool is_service() const { return false; }

 public:
   int16_t *get_idset();
   uint16_t size() const { return size_; };
   uint16_t max_size() const { return max_size_; }
   bool hash();
   connection::Basic *get(uint16_t id);
   connection::Basic *get(std::string &name) {
     if (connection_names_.find(name) == connection_names_.end())
       return nullptr;
     return get(connection_names_[name]);
   };
   connection::Pool *get_pool();
   int32_t get_onestep_accept() const;
   void set_onestep_accept(int32_t count);
   uint64_t get_send_bytes();
   uint64_t get_receive_bytes();
   bool is_ready() const { return ready_; };
   bool full() const { return pool_ ? pool_->full() : true; };

 public: //Packet queue, can work in mutli thread.
   virtual bool send(packet::Interface *packet, 
                     uint16_t id, 
                     uint32_t flag = kPacketFlagNone);
   virtual bool process_command_cache();
   virtual bool recv(packet::Interface *&packet,
                     uint16_t &connectionid,
                     uint32_t &flag);
   virtual void on_disconnect(connection::Basic *) {}
   virtual void on_connect(connection::Basic *) {}
   bool cache_resize();
   void broadcast(packet::Interface *packet);

 public:
   void callback_disconnect(
       std::function<void (connection::Basic *)> callback) {
     callback_disconnect_ = callback;
   }

   void callback_connect(std::function<void (connection::Basic *)> callback) {
     callback_connect_ = callback;
   }

   void set_connection_name(uint16_t id, std::string &name) {
     connection_names_[name] = id;
   }

 public:
   bool checkpool(bool log = true);

 public:
   std::thread::id thread_id() const { return thread_id_; }

 public:
   protocol::Interface *protocol() { return protocol_default(); };
   static protocol::Interface *protocol_default() {
     static protocol::Basic proto;
     return &proto;
   };

 public:
   virtual bool select() = 0;             /* 网络侦测 */
   virtual bool process_input() = 0;      /* 数据接收接口 */
   virtual bool process_output() = 0;     /* 数据发送接口 */
   virtual bool process_exception() = 0;  /* 异常连接处理 */
   virtual bool process_command() = 0;    /* 消息执行 */

 public:
   //For listener.
   virtual connection::Basic *accept() { return nullptr; };
   virtual int32_t listener_socket_id() const { return SOCKET_INVALID; };

 protected:
   uint16_t connection_max_size_;
   int32_t fdsize_; //实际的网络连接数量，正在连接的，
                    //其实和count_一样，不过此值只用于轮询模式
   bool ready_; /* 是否把该准备的已经准备好了，主要是内存的初始化 */

 protected:
   int16_t *connection_idset_;    /* 连接的ID数组 */
   uint16_t max_size_;            /* 连接的最大数量 */
   uint16_t size_;                /* 连接的当前数量 */
   uint64_t send_bytes_;          /* 发送字节数 */
   uint64_t receive_bytes_;       /* 接收字节数 */
   int32_t onestep_accept_;       /* 帧内接受的新连接数量, -1无限制 */
   std::unique_ptr< connection::Pool > pool_;   /* 连接池 */
   /* 连接时的回调，所有管理器内的连接都会触发 */
   std::function<void (connection::Basic *)> callback_disconnect_;
   /* 断开连接的回调，同上 */
   std::function<void (connection::Basic *)> callback_connect_;
   cache_t cache_;
   std::map<std::string, uint16_t> connection_names_; //The connection name to id.
   std::mutex mutex_;

 private:
   std::thread::id thread_id_;

};

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif //PF_NET_CONNECTION_MANAGER_BASE_H_
