/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id factory.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/04/01 16:50
 * @uses The db factory.
 *       getenv的指针不能在外部被释放，可以通过closeenv来操作。
*/
#ifndef PLAIN_SCRIPT_FACTORY_H_
#define PLAIN_SCRIPT_FACTORY_H_

#include "plain/script/config.h"
#include <unordered_map>

namespace plain::script {

class PLAIN_API Factory {

 public:
   Factory() : last_del_eid_{SCRIPT_EID_INVALID} {};
   ~Factory() {};

 public:
   eid_t newenv(const config_t &config);
   eid_t newenv(Interface *env);
   Interface *getenv(eid_t eid) {
     if (envs_[eid]) return envs_[eid].get();
     return nullptr;
   }
   void closeenv(eid_t eid) {
     auto it = envs_.find(eid);
     if (it != envs_.end()) envs_.erase(it);
     last_del_eid_ = eid; 
   }

 private:
   eid_t neweid();

 private:
   std::unordered_map<eid_t, std::unique_ptr<Interface>> envs_;
   eid_t last_del_eid_;
   std::mutex mutex_;

};

} //namespace plain::script

#endif //PLAIN_SCRIPT_FACTORY_H_
