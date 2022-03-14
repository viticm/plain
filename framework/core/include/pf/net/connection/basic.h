/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id basic.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/07/04 19:30
 * @uses net connect information
 *       cn:
 *         现在的输入流解压缩的流程如下：从输入流中读取所有数据到压缩缓存中，
 *         解压数据到缓存中，并将解压后的缓存数据写入到输入流中（异于输出流）。
 *         这种模式，可以考虑优化 2015-2-16 viticm
 */
#ifndef PF_NET_CONNECTION_BASE_H_
#define PF_NET_CONNECTION_BASE_H_

#include "pf/net/connection/config.h"
#include "pf/basic/type/variable.h"
#include "pf/net/packet/interface.h"
#include "pf/net/socket/basic.h"
#include "pf/net/protocol/interface.h"
#include "pf/net/connection/manager/config.h"
#include "pf/net/stream/input.h"
#include "pf/net/stream/output.h"

#define NET_CONNECTION_COMPRESS_BUFFER_SIZE (1024 * 1024)
#define NET_CONNECTION_UNCOMPRESS_BUFFER_SIZE \
  (NET_CONNECTION_COMPRESS_BUFFER_SIZE + \
   NET_CONNECTION_COMPRESS_BUFFER_SIZE/16 + 64 + 3 + \
   NET_STREAM_COMPRESSOR_HEADER_SIZE)

struct packet_async_t {
  pf_net::packet::Interface *packet;
  uint16_t packetid;
  uint32_t flag;
  packet_async_t() : 
    packet{nullptr},
    packetid{0},
    flag{kPacketFlagNone} {
  }

  ~packet_async_t() {
    safe_delete(packet);
    packetid = 0;//ID_INVALID;
    flag = kPacketFlagNone;
  }
};

namespace pf_net {

namespace connection {

class PF_API Basic {

 public:
   Basic();
   virtual ~Basic();

 public:
   virtual bool init(protocol::Interface *protocol); //初始化，主要是socket

 public:
   virtual bool process_input();
   virtual bool process_output();
   virtual bool process_command();
   virtual bool heartbeat(uint64_t time = 0, uint32_t flag = 0);
   virtual bool send(packet::Interface *packet);
   virtual bool routing(const std::string &name, 
                        packet::Interface *packet, 
                        const std::string &service = "");
   virtual bool forward(packet::Interface *packet);

 public:
   //! 建议不使用此方式，该接口很可能在将来被删除
   //支持自定义连接包执行的接口(不过这种方式外部需重写connection,后考虑优化)
   using packet_execute_function = 
     std::function<uint32_t(connection::Basic *, packet::Interface *)>;
   virtual packet_execute_function get_packet_execute_function() {
     return nullptr;
   }

 public:
   virtual void disconnect();
   virtual void on_connect();
   virtual void on_disconnect();
 
 public:
   void callback_connect(std::function<void (Basic *)> callback);
   void callback_disconnect(std::function<void (Basic *)> callback); 

 public:
   int16_t get_id() const { return id_; }
   void set_id(int16_t id) { id_ = id; }
   int16_t get_managerid() const { return managerid_; }
   void set_managerid(int16_t managerid) { managerid_ = managerid; }
   socket::Basic *socket() { return socket_.get(); }
   // Exit the connection(safe with the manager connection).
   void exit();
   bool empty() const { return empty_; }
   void set_empty(bool status = true) { empty_ = status; }
   bool is_disconnect() const { return disconnect_; }
   void set_disconnect(bool status = true) { disconnect_ = status; }
   bool is_valid();
   void clear();
   bool ready() const { return ready_; } //connection is ready then is true;
   uint8_t get_execute_count_pretick() const { return execute_count_pretick_; }
   void set_execute_count_pretick(uint8_t count) { 
     execute_count_pretick_ = count; 
   }
   void set_status(uint8_t status) { status_ = status; }
   uint8_t get_status() const { return status_; }
   void set_safe_encrypt(bool flag) { safe_encrypt_ = flag; }
   bool is_safe_encrypt() const { return safe_encrypt_; }
   bool check_safe_encrypt() const {
     return safe_encrypt_ || 0 == safe_encrypt_time_;
   }
   void set_safe_encrypt_time(uint32_t time) { safe_encrypt_time_ = time; }
   bool is_safe_encrypt_timeout() const;
   void set_param(const std::string &_name, 
                  const pf_basic::type::variable_t &value) {
     params_[_name] = value;
   }
   pf_basic::type::variable_t get_param(const std::string &_name) {
     return params_[_name];
   }
   bool connect(const std::string &ip, uint16_t port);

 public:
   void compress_set_mode(compress_mode_t mode);
   compress_mode_t compress_get_mode() const { return compress_mode_; }
   void encrypt_enable(bool enable);
   void encrypt_set_key(const char *key);
   uint32_t get_receive_bytes();
   uint32_t get_send_bytes();

 public:
   stream::Input &istream() { return *istream_.get(); }
   stream::Output &ostream() { return *ostream_.get(); }
   stream::Input &istream_compress() { return *istream_compress_.get(); }
   int8_t packet_index() { return packet_index_++; }
   void set_protocol(protocol::Interface *_protocol) {
     protocol_ = _protocol;
   }
   protocol::Interface *protocol() {
     return protocol_;
   }
   void set_manager(manager::Interface *manager) {
     manager_ = manager;
   }
   manager::Interface *get_manager() {
     return manager_;
   }
   const std::string name() const {
     return name_;
   }
   void set_name(const std::string &_name) {
     name_ = _name;
   }
   void set_routing(const std::string &_name, bool flag) {
     routing_list_[_name] = flag == true ? 1 : -1;
   }

   // Set the error times(now use in protocol peek).
   void set_error_times(uint8_t times) {
     error_times_ = times;
   }

   // Get the error times.
   uint8_t get_error_times() const {
     return error_times_;
   }

   // Increase error times.
   void inc_error_times() {
     error_times_ += 1;
   }

 private:
   void process_input_compress();

 private:
   int16_t id_;
   int16_t managerid_;
   std::unique_ptr<socket::Basic> socket_;
   std::unique_ptr<stream::Input> istream_;
   std::unique_ptr<stream::Input> istream_compress_;
   std::unique_ptr<stream::Output> ostream_;
   protocol::Interface *protocol_; //用个引用来做是否好些？
   manager::Interface *manager_;

 private:
   bool empty_;
   bool disconnect_;
   bool ready_;

 private:
   compress_mode_t compress_mode_;
   char *uncompress_buffer_;
   char *compress_buffer_;

 private:
   uint32_t receive_bytes_;
   uint32_t send_bytes_;

 private:
   int8_t packet_index_;
   uint8_t execute_count_pretick_;
   uint8_t status_;
   bool safe_encrypt_; //This flag say the connection if encrypt in encrypt mode.
   uint32_t safe_encrypt_time_; //If not 0 then will check the safe encrypt. 
   std::string name_; //The connection name.
   pf_basic::type::variable_set_t params_; //The extend param set.
   std::map<std::string, int8_t> routing_list_; //The connection routing list.
   std::mutex mutex_;
   uint8_t error_times_;

};

} //namespace connection

} //namespace pap_common_net

#endif //PF_NET_CONNECTION_BASE_H_
