/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id thread.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/14 09:28
 * @uses The engine default thread module.
*/
#ifndef PF_ENGINE_THREAD_H_
#define PF_ENGINE_THREAD_H_

#include "pf/engine/config.h"
#include "pf/script/config.h"
#include "pf/db/config.h"
#include "pf/cache/config.h"
#include "pf/net/connection/manager/config.h"

namespace pf_engine {

namespace thread {

bool for_net(pf_net::connection::manager::Basic *net);
bool for_db(pf_db::Interface *db);
bool for_cache(pf_cache::Manager *cache);
bool for_script(pf_script::Interface *env);

} //namespace thread

} //namespace pf_engine

#endif //PF_ENGINE_THREAD_H_
