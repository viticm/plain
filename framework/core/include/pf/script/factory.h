/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id factory.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/10 19:39
 * @uses The db factory.
 *       getenv的指针不能在外部被释放，可以通过closeenv来操作。
*/
#ifndef PF_SCRIPT_FACTORY_H_
#define PF_SCRIPT_FACTORY_H_

#include "pf/script/config.h"

namespace pf_script {

class PF_API Factory {

 public:
   Factory() : last_del_eid_{SCRIPT_EID_INVALID} {};
   ~Factory() {};

 public:
   eid_t newenv(const config_t &config);
   eid_t newenv(Interface *env);
   Interface *getenv(eid_t eid) {
     if (envs_[eid]) return envs_[eid].get();
     return nullptr;
   };
   void closeenv(eid_t eid) {
     auto it = envs_.find(eid);
     if (it != envs_.end()) envs_.erase(it);
     last_del_eid_ = eid; 
   };

 private:
   eid_t neweid();

 private:
   std::map< eid_t, std::unique_ptr< Interface > > envs_;
   eid_t last_del_eid_;
   std::mutex mutex_;

};

}; //namespace pf_script

#endif //PF_SCRIPT_FACTORY_H_
