#include "pf/basic/io.tcc"
#include "pf/basic/time_manager.h"
#include "pf/basic/logger.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/manager/connector.h"
#include "pf/db/interface.h"
#include "pf/db/factory.h"
#include "pf/script/factory.h"
#include "pf/script/interface.h"
#include "pf/cache/repository.h"
#include "pf/cache/db_store.h"
#include "pf/cache/manager.h"
#include "pf/sys/thread.h"
#include "pf/engine/thread.h"
#include "pf/file/library.h"
#include "pf/engine/kernel.h"

using namespace pf_engine;

template <> Kernel *pf_basic::Singleton< Kernel >::singleton_ = nullptr;

Kernel *Kernel::getsingleton_pointer() {
  return singleton_;
}

Kernel &Kernel::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

Kernel::Kernel() :
  net_{nullptr},
  db_factory_{nullptr},
  db_eid_{DB_EID_INVALID},
  cache_{nullptr},
  script_factory_{nullptr},
  script_eid_{SCRIPT_EID_INVALID},
  isinit_{false},
  stop_{false} {
}

Kernel::~Kernel() {
  for (std::thread &worker : thread_workers_) {
    worker.join();
  }
}

pf_db::Interface *Kernel::get_db() {
  if (is_null(db_factory_)) return nullptr;
  auto env = db_factory_->getenv(db_eid_);
  return env;
}

pf_script::Interface *Kernel::get_script() {
  if (is_null(script_factory_)) return nullptr;
  auto env = script_factory_->getenv(script_eid_);
  return env;
}

bool Kernel::init() {
  if (isinit_) return true;
  if (!init_base()) return false;
  if (!init_net()) return false;
  if (!init_db()) return false;
  if (!init_cache()) return false;
  if (!init_script()) return false;
  SLOW_DEBUGLOG(ENGINE_MODULENAME, "[%s] Kernel::init ok!", ENGINE_MODULENAME);
  return true;
}

void Kernel::run() {
  if (!is_null(net_)) {
    auto net = net_.get();
    this->newthread([&net]() { return thread::for_net(net); });
  }
  if (!is_null(db_factory_) && db_eid_ != DB_EID_INVALID) {
    auto env = db_factory_->getenv(db_eid_);
    this->newthread([&env]() { return thread::for_db(env); });
  }
  if (!is_null(script_factory_) && script_eid_ != SCRIPT_EID_INVALID) { 
    auto env = script_factory_->getenv(script_eid_);
    env->call(GLOBALS["default.script.enter"].data);
    this->newthread([&env]() { return thread::for_script(env); });
  }
  if (!is_null(cache_)) {
    auto cache = cache_.get();
    this->newthread([&cache]() { return thread::for_cache(cache); });
  }
  GLOBALS["app.status"] = kAppStatusRunning;
  loop();
}

void Kernel::stop() {
  for (std::thread &worker : thread_workers_) {
    pf_sys::thread::stop(worker);
  }
  GLOBALS["app.status"] = kAppStatusStop;
  stop_ = true;
}

bool Kernel::init_base() {
  using namespace pf_basic;
 
  //Time manager.
  if (is_null(TIME_MANAGER_POINTER)) {
    auto time_manager = new TimeManager();
    if (is_null(time_manager)) return false;
    unique_move(TimeManager, time_manager, g_time_manager);
    if (!g_time_manager->init()) return false;
  }

  //Logger.
  if (is_null(LOGSYSTEM_POINTER)) {
    auto logger = new Logger();
    if (is_null(logger)) return false;
    unique_move(Logger, logger, g_logger);
  }

  if (is_null(LIBRARY_MANAGER_POINTER)) {
    auto librarymanager = new pf_file::LibraryManager;
    unique_move(pf_file::LibraryManager, librarymanager, g_librarymanager); 
  }

  if (LIBRARY_MANAGER_POINTER) {
    auto it = library_load_.begin();
    for (; it != library_load_.end(); ++it) {
      if (!LIBRARY_MANAGER_POINTER->load(it->first, it->second))
        return false;
    }
  }
  
  return true;
}

bool Kernel::init_net() {
  using namespace pf_net;
  using namespace pf_basic;
  if (GLOBALS["default.net.open"] == false) return true;
  SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                "[%s] Kernel::init_net start...", 
                ENGINE_MODULENAME);
  connection::manager::Basic *net{nullptr};
  auto conn_max = GLOBALS["default.net.conn_max"].get<uint16_t>();
  if (GLOBALS["default.net.service"] == true) {
    net = new connection::manager::Listener();
    unique_move(connection::manager::Basic, net, net_)
    auto service_ip = GLOBALS["default.net.service_ip"].c_str();
    auto service_port = GLOBALS["default.net.service_port"].get<uint16_t>();
    auto service = dynamic_cast< connection::manager::Listener *>(net);
    if (!service->init(conn_max, service_port, service_ip)) return false;
    std::string host{service->host()};
    SLOW_DEBUGLOG(ENGINE_MODULENAME,
                  "[%s] service listen at: host[%s] port[%d] connections[%d].",
                  ENGINE_MODULENAME,
                  0 == host.size() ? "*" : host.c_str(),
                  service->port(),
                  conn_max);
  } else {
    net = new connection::manager::Connector();
    unique_move(connection::manager::Basic, net, net_)
    if (!net->init(conn_max)) return false;
  }
  return true;
}

bool Kernel::init_db() {
  using namespace pf_db;
  if (GLOBALS["default.db.open"] == false) return true;
  SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                "[%s] Kernel::init_db start...", 
                ENGINE_MODULENAME);
  auto factory = new Factory();
  if (is_null(factory)) return false;
  unique_move(Factory, factory, db_factory_);
  config_t conf;
  conf.type = GLOBALS["default.db.type"].get<int8_t>();
  conf.name = GLOBALS["default.db.name"].c_str();
  conf.username = GLOBALS["default.db.user"].c_str();
  conf.password = GLOBALS["default.db.password"].c_str();
  db_eid_ = db_factory_->newenv(conf);
  if (DB_EID_INVALID == db_eid_) return false;
  auto env = db_factory_->getenv(db_eid_);
  if (!env->init()) return false;
  return true;
}

bool Kernel::init_cache() {
  using namespace pf_cache;
  if (GLOBALS["default.cache.open"] == false) return true;
  SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                "[%s] Kernel::init_cache start...", 
                ENGINE_MODULENAME);
  auto cache = new Manager();
  if (is_null(cache)) return false;
  unique_move(Manager, cache, cache_)
  auto dirver = cache->create_db_dirver();
  if (is_null(dirver)) return false;
  auto store = dynamic_cast< DBStore *>(dirver->store());
  auto key_map = GLOBALS["default.cache.key_map"].get<int32_t>();
  auto recycle_map = GLOBALS["default.cache.recycle_map"].get<int32_t>();
  auto query_map = GLOBALS["default.cache.query_map"].get<int32_t>();
  store->set_key(key_map, recycle_map, query_map);
  store->set_service(GLOBALS["default.cache.service"].get<bool>());
  if (!store->load_config(GLOBALS["default.cache.conf"].c_str())) return false;
  if (!store->init()) return false;
  return true;
}

bool Kernel::init_script() {
  using namespace pf_script;
  if (GLOBALS["default.script.open"] == false) return true;
  SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                "[%s] Kernel::init_script start...", 
                ENGINE_MODULENAME);
  auto factory = new Factory();
  if (is_null(factory)) return false;
  unique_move(Factory, factory, script_factory_);
  config_t conf;
  conf.rootpath = GLOBALS["default.script.rootpath"].c_str();
  conf.workpath = GLOBALS["default.script.workpath"].c_str();
  conf.type = GLOBALS["default.script.type"].get<int8_t>();
  script_eid_ = script_factory_->newenv(conf);
  if (SCRIPT_EID_INVALID == script_eid_) return false;
  auto env = script_factory_->getenv(script_eid_);
  if (!env->init()) return false;
  if (!env->bootstrap(GLOBALS["default.script.bootstrap"].c_str())) 
    return false;
  return true;
}

void Kernel::loop() {
  for (;;) {
    if (GLOBALS["app.status"] == kAppStatusStop) break;
    auto starttime = TIME_MANAGER_POINTER->get_tickcount();
    std::function<void()> task;
    {
      if (!tasks_.empty()) {
        task = std::move(this->tasks_.front());
        this->tasks_.pop();
      }
    }
    if (task) task();
    worksleep(starttime);
  }
  auto check_starttime = TIME_MANAGER_POINTER->get_tickcount();
  for (;;) {
    auto diff_time = TIME_MANAGER_POINTER->get_tickcount() - check_starttime;
    if (diff_time / 1000 > 30) {
      pf_basic::io_cerr("[%s] wait thread exit exceed 30 seconds.", 
                        GLOBALS["app.name"].c_str());
      break;
    }
    if (pf_sys::ThreadCollect::count() <= 0) break;
  }
}
