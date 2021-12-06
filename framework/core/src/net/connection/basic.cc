#include "pf/basic/logger.h"
#include "pf/basic/type/variable.h"
#include "pf/basic/io.tcc"
#include "pf/basic/time_manager.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/packet/forward.h"
#include "pf/net/packet/routing.h"
#include "pf/net/packet/routing_lost.h"
#include "pf/engine/kernel.h"
#include "pf/script/interface.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/basic.h"

namespace pf_net {

namespace connection {

Basic::Basic() : 
  id_{ID_INVALID},
  managerid_{ID_INVALID},
  socket_{nullptr},
  istream_{nullptr},
  istream_compress_{nullptr},
  ostream_{nullptr},
  protocol_{nullptr},
  listener_{nullptr},
  empty_{true},
  disconnect_{false},
  ready_{false},
  compress_mode_{kCompressModeNone},
  uncompress_buffer_{nullptr},
  compress_buffer_{nullptr},
  receive_bytes_{0},
  send_bytes_{0},
  packet_index_{0},
  execute_count_pretick_{NET_CONNECTION_EXECUTE_COUNT_PRE_TICK_DEFAULT},
  status_{0},
  safe_encrypt_{false},
  safe_encrypt_time_{0},
  name_{""},
  error_times_{0} {
  //do nothing
}

Basic::~Basic() {
  safe_delete(compress_buffer_);
  safe_delete(uncompress_buffer_);
}

bool Basic::init(protocol::Interface *_protocol) {
  if (ready()) return true; 
  if (is_null(_protocol)) return false;
  std::unique_ptr<socket::Basic> _socket(new socket::Basic());
  protocol_ = _protocol;
  socket_ = std::move(_socket);
  Assert(socket_.get());
  std::unique_ptr<stream::Input> _istream(
      new stream::Input(socket_.get(),
      NETINPUT_BUFFERSIZE_DEFAULT,
      64 * 1024 * 1024));
  istream_ = std::move(_istream);
  Assert(istream_.get());
  istream_->init();
  std::unique_ptr<stream::Output> _ostream (
      new stream::Output(
      socket_.get(),
      NETOUTPUT_BUFFERSIZE_DEFAULT,
      64 * 1024 * 1024));
  ostream_ = std::move(_ostream);
  Assert(ostream_.get());
  ostream_->init();
  ready_ = true;
  return true;
}

bool Basic::process_input() {
  bool result = false;
  if (is_disconnect()) return true;
  pf_util::compressor::Assistant *assistant = nullptr;
  assistant = istream_->getcompressor()->getassistant();    
  try {
    int32_t fillresult = SOCKET_ERROR;
    if (assistant->isenable()) {
      if (is_null(istream_compress_)) {
        SLOW_ERRORLOG("net",
                      "[net.connection] (Basic::process_input)"
                      " the socket compress stream is null.");
        return false;
      }
      fillresult = istream_compress_->fill();
    } else {
      fillresult = istream_->fill();
    }

    if (fillresult <= SOCKET_ERROR) {
      char errormessage[FILENAME_MAX];
      memset(errormessage, '\0', sizeof(errormessage));
      istream_->socket()->get_last_error_message(
          errormessage, 
          static_cast<uint16_t>(sizeof(errormessage) - 1));
      SLOW_ERRORLOG("net",
                    "[net.connection] (Basic::process_input)"
                    " socket_inputstream_->fill() result: %d %s",
                    fillresult,
                    errormessage);
      result = false;
    } else {
      result = true;
      receive_bytes_ += static_cast<uint32_t>(fillresult); //网络流量
    }
  } catch(...) {
    SaveErrorLog();
  }
  
  if (assistant->isenable() && !is_null(istream_compress_))
    process_input_compress(); //compress
  return result;
}

void Basic::process_input_compress() {
  if (is_null(protocol_)) return;
  protocol_->compress(this, uncompress_buffer_, compress_buffer_);
}

bool Basic::process_output() {
  bool result = false;
  if (is_disconnect()) return true;
  try {
    int32_t flushresult = ostream_->flush();
    if (flushresult <= SOCKET_ERROR) {
      char errormessage[FILENAME_MAX] = {0};
      istream_->socket()->get_last_error_message(
          errormessage, 
          static_cast<uint16_t>(sizeof(errormessage) - 1));
      SLOW_ERRORLOG("net",
                    "[net.connection] (Basic::processoutput)"
                    " socket_outputstream_->flush() result: %d %s",
                    flushresult,
                    errormessage);
      result = false;
    } else {
      result = true;
      send_bytes_ += static_cast<uint32_t>(flushresult);
    }
  } catch(...) {
    SaveErrorLog();
  }
  return result;
}

bool Basic::process_command() {
  if (is_null(protocol_)) return false;
  return protocol_->command(this, execute_count_pretick_);
}

bool Basic::send(packet::Interface *packet) {
  std::unique_lock<std::mutex> autolock(mutex_);
  if (is_disconnect()) return false;
  if (is_null(protocol_)) return false;
  return protocol_->send(this, packet);
}

bool Basic::heartbeat(uint32_t, uint32_t) {
  using namespace pf_basic;
  auto now = TIME_MANAGER_POINTER->get_ctime();
  if (is_disconnect()) return false;
  if (safe_encrypt_time_ != 0 && !is_safe_encrypt()) {
    now = TIME_MANAGER_POINTER->get_ctime();
    if (safe_encrypt_time_ + NET_ENCRYPT_CONNECTION_TIMEOUT < now) {
      io_cwarn("[%s] Connection with safe encrypt timeout!",
               NET_MODULENAME);
      return false;
    }
  }
  // Routing check also can put it in input.
  std::string aim_name = params_["routing"].data;
  if (aim_name != "") {
    auto last_check = params_["routing_check"].get<uint32_t>();
    if (now - last_check >= 3) {
      std::string service = params_["routing_service"].data;
      manager::Listener *listener = ENGINE_POINTER->get_service(service);
      if (is_null(listener)) return false;
      auto connection = listener->get(aim_name);
      if (is_null(connection) || connection->is_disconnect()) {
        io_cwarn("[%s] routing(%s|%s) lost!", 
                 NET_MODULENAME, 
                 service.c_str(), 
                 aim_name.c_str());
        return false;
      }
      params_["routing_check"] = now;
    }
  }
  return true;
}

void Basic::disconnect() {
  using namespace pf_basic::type;
  //Notice routing original.
  std::string aim_name = params_["routing"].data;
  if (aim_name != "") {
    std::string service = params_["routing_service"].data;
    manager::Listener *listener = ENGINE_POINTER->get_service(service);
    if (is_null(listener)) return;
    auto connection = listener->get(aim_name);
    if (!is_null(connection) && name_ != "") {
      packet::RoutingLost packet;
      packet.set_aim_name(name_);
      connection->send(&packet);
    }
  }
  auto script = ENGINE_POINTER->get_script();
  if (!is_null(script) && GLOBALS["default.script.netlost"] != "") {
    auto func = GLOBALS["default.script.netlost"].data;
    variable_array_t params;
    params.emplace_back(this->name());
    params.emplace_back(this->get_id());
    variable_array_t results;
    script->call(func, params, results);
  }
  clear();
}

void Basic::exit() {
  if (!is_null(listener_)) {
    listener_->remove(this);
  } else {
    disconnect();
  }
}

bool Basic::is_valid() {
  if (is_null(socket_.get())) return false;
  bool result = false;
  result = socket_->is_valid();
  return result;
}

void Basic::clear() {
  if (socket_) socket_->close();
  if (istream_) istream_->clear();
  if (ostream_) ostream_->clear();
  set_managerid(ID_INVALID);
  packet_index_ = 0;
  status_ = 0;
  execute_count_pretick_ = NET_CONNECTION_EXECUTE_COUNT_PRE_TICK_DEFAULT;
  set_disconnect(true);
  set_empty(true);
  set_safe_encrypt(false);
  safe_encrypt_time_ = 0;
  name_ = "";
  params_.clear();
  routing_list_.clear();
  error_times_ = 0;
}

uint32_t Basic::get_receive_bytes() {
  uint32_t result = receive_bytes_;
  receive_bytes_ = 0;
  return result;
  return 0;
}

uint32_t Basic::get_send_bytes() {
  uint32_t result = send_bytes_;
  send_bytes_ = 0;
  return result;
}

void Basic::compress_set_mode(compress_mode_t mode) {
  compress_mode_ = mode;
  pf_util::compressor::Assistant *assistant = nullptr;
  bool inputstream_compress_enable = 
    kCompressModeInput == compress_get_mode() || 
    kCompressModeAll == compress_get_mode() ? true : false;
  bool outputstream_compress_enable = 
    kCompressModeOutput == compress_get_mode() || 
    kCompressModeAll == compress_get_mode() ? true : false;
  assistant = istream_->getcompressor()->getassistant();    
  assistant->enable(inputstream_compress_enable);
  if (assistant->isenable()) {
    if (is_null(uncompress_buffer_)) {
      uncompress_buffer_ = new char[NET_CONNECTION_UNCOMPRESS_BUFFER_SIZE];
      memset(uncompress_buffer_, 0, NET_CONNECTION_UNCOMPRESS_BUFFER_SIZE);
    }
    if (is_null(compress_buffer_)) {
      compress_buffer_ = new char[NET_CONNECTION_COMPRESS_BUFFER_SIZE];
      memset(compress_buffer_, 0, NET_CONNECTION_COMPRESS_BUFFER_SIZE);
    }
    if (is_null(istream_compress_)) {
      std::unique_ptr<stream::Input> _istream_compress(
            new stream::Input(socket_.get(),
            NETINPUT_BUFFERSIZE_DEFAULT,
            64 * 1024 * 1024));
      istream_compress_ = std::move(_istream_compress);
    }
  }
  assistant = ostream_->getcompressor()->getassistant();
  assistant->enable(outputstream_compress_enable);
}

void Basic::encrypt_enable(bool enable) {
  istream_->encryptenable(enable);
  ostream_->encryptenable(enable);
}

void Basic::encrypt_set_key(const char *key) {
  istream_->encrypt_setkey(key);
  ostream_->encrypt_setkey(key);
}

bool Basic::routing(const std::string &_name, 
                    packet::Interface *packet,
                    const std::string &service) {
  if (routing_list_[_name] != 1 || is_null(packet)) return false;
  packet::Routing routing_packet;
  routing_packet.set_destination(service);
  routing_packet.set_aim_name(_name);
  routing_packet.set_packet_size(packet->size());
  return send(&routing_packet) && send(packet);
}

bool Basic::forward(packet::Interface *packet) {
  if (is_null(packet)) return false;
  std::string aim_name = params_["routing"].data;
  if (aim_name == "") return false;
  std::string service = params_["routing_service"].data;
  if (service == "") service = "default";
  auto listener = ENGINE_POINTER->get_listener(service);
  if (is_null(listener)) return false;
  auto connection = listener->get(aim_name);
  if (is_null(connection) || connection->is_disconnect()) return false;
  packet::Forward forward_packet;
  forward_packet.set_original(name());
  forward_packet.set_packet_size(packet->size());
  return connection->send(&forward_packet) && connection->send(packet);
}

} //namespace connection

} //namespace pf_net
