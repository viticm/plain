/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id listener_factory.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/11/22 19:14
 * @uses The net listener connection manager factory.
 *       getenv的指针不能在外部被释放，可以通过closeenv来操作。
*/
#ifndef PF_NET_CONNECTION_MANAGER_LISTENER_FACTORY_H_
#define PF_NET_CONNECTION_MANAGER_LISTENER_FACTORY_H_

#include "pf/net/connection/manager/config.h"

namespace pf_net {

namespace connection {

namespace manager {

class PF_API ListenerFactory {

 public:
   ListenerFactory() : last_del_eid_{NET_EID_INVALID} {}
   ~ListenerFactory() {}

 public:
   eid_t newenv(const listener_config_t &config);
   eid_t newenv(Listener *env);
   Listener *getenv(eid_t eid) {
     if (envs_[eid]) return envs_[eid].get();
     return nullptr;
   }
   void closeenv(eid_t eid) {
     auto it = envs_.find(eid);
     if (it != envs_.end()) envs_.erase(it);
     last_del_eid_ = eid;
   }
   size_t size() const {
     return envs_.size();
   }

 private:
   eid_t neweid();

 private:
   std::map< eid_t, std::unique_ptr< Listener > > envs_;
   eid_t last_del_eid_;
   std::mutex mutex_;

};

} //namespace manager

} //namespace connection

} //namespace pf_net

#endif //PF_NET_CONNECTION_MANAGER_LISTENER_FACTORY_H_
