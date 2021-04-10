#include "pf/net/connection/manager/basic.h"
#include "pf/db/interface.h"
#include "pf/script/interface.h"
#include "pf/cache/repository.h"
#include "pf/cache/db_store.h"
#include "pf/cache/manager.h"
#include "pf/sys/thread.h"
#include "pf/console/application.h"
#include "pf/engine/thread.h"

namespace pf_engine {

namespace thread {

//Also can use object thread like std::thread t(&O::func, this, ...).
//Or by pf_sys::thread::start(t, &O::func, this, ...).

bool for_net(pf_net::connection::manager::Basic *net) {
  auto stime = TIME_MANAGER_POINTER->get_tickcount();
  if (is_null(net)) return false;
  net->tick();
  auto etime = TIME_MANAGER_POINTER->get_tickcount();
  auto dtime = etime - stime;
  if (dtime > 20) {
    SLOW_ERRORLOG("engine", "Thread for_net delay time: %ld", dtime);
  }
  return true;
}

bool for_db(pf_db::Interface *db) {
  using namespace pf_sys;
  if (is_null(db)) return false;
  db->check_db_connect();
  return true;
}

bool for_cache(pf_cache::Manager *cache) {
  if (is_null(cache)) return false;
  auto dirver = cache->get_db_dirver();
  auto store = dynamic_cast<pf_cache::DBStore *>(dirver->store());
  if (is_null(store)) return false;
  store->tick();
  return true;
}

bool for_script(pf_script::Interface *env) {
  auto stime = TIME_MANAGER_POINTER->get_tickcount();
  using namespace pf_script;
  if (is_null(env)) return false;
  env->task_queue()->work_one();
  if (GLOBALS["default.script.heartbeat"] != "") 
    env->call(GLOBALS["default.script.heartbeat"].data);
  auto time = 
    static_cast<int32_t>(1000 / GLOBALS["default.engine.frame"].get<int32_t>());
  env->gccheck(time);
  auto etime = TIME_MANAGER_POINTER->get_tickcount();
  auto dtime = etime - stime;
  if (dtime > 20) {
    SLOW_ERRORLOG("engine", "Thread for_script delay time: %ld", dtime);
  }
  return true;
}

bool for_console(pf_console::Application *console) {
  if (is_null(console)) return false;
  /**
  if (!feof(stdin)) {
    char input_str[512]{0};
    fgets(input_str, 512, stdin);
    if (strlen(input_str) > 0) {
      std::cout << "input_str: " << input_str << std::endl;
    } 
  }
  **/
  return true;
}

} //namespace thread

} //namespace pf_engine
