#include "pf/basic/logger.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/manager/listener_factory.h"

using namespace pf_net::connection::manager;

eid_t ListenerFactory::newenv(const listener_config_t &config) {
  eid_t eid = neweid();
  if (NET_EID_INVALID == eid) return eid;
  std::unique_ptr< Listener > pointer(new Listener);
  if (is_null(pointer) || 
      !pointer->init(config.conn_max, config.port, config.ip)) {
    last_del_eid_ = eid;
    return NET_EID_INVALID;
  }
  pointer->set_name(config.name); 
  if (config.encrypt_str != "") 
    pointer->set_safe_encrypt_str(config.encrypt_str);
  envs_[eid] = std::move(pointer);
  return eid;
}

eid_t ListenerFactory::newenv(Listener *env) {
  eid_t eid = neweid();
  if (NET_EID_INVALID == eid) return eid;
  std::unique_ptr< Listener > pointer(env);
  envs_[eid] = std::move(pointer);
  return eid;
}

eid_t ListenerFactory::neweid() {
  std::unique_lock<std::mutex> autolock(mutex_);
  eid_t eid = NET_EID_INVALID;
  eid = NET_EID_INVALID == last_del_eid_ ?
        static_cast<eid_t>(envs_.size() + 1) : last_del_eid_;
  last_del_eid_ = NET_EID_INVALID;
  return eid;
}
