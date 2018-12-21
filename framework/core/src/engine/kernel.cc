#include "pf/basic/io.tcc"
#include "pf/basic/time_manager.h"
#include "pf/basic/base64.h"
#include "pf/basic/string.h"
#include "pf/basic/logger.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/manager/listener_factory.h"
#include "pf/net/connection/manager/connector.h"
#include "pf/net/packet/handshake.h"
#include "pf/net/packet/register_connection_name.h"
#include "pf/db/interface.h"
#include "pf/db/null.h"
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

pf_db::Interface *db_null_env_creator() { 
  return new pf_db::Null();
}

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

pf_net::connection::Basic *Kernel::get_connector(const std::string &name) {
  if (is_null(net_connector_)) return nullptr;
  auto connection = net_connector_->get(name);
  if (!is_null(connection)) return connection;
  if (connect_list_.find(name) == connect_list_.end() ||
      -1 == connect_list_[name]) return nullptr;
  int16_t id = connect_list_[name];
  return net_connector_->get(id);
}

 pf_net::connection::manager::Listener *Kernel::get_listener(
     const std::string &name) {
  if (is_null(net_listener_factory_) || 
      listen_list_.find(name) == listen_list_.end()) {
    return nullptr;
  }
  return net_listener_factory_->getenv(listen_list_[name]);
}


pf_net::connection::manager::Listener *Kernel::get_service(
    const std::string &name) {
  using namespace pf_net::connection::manager;
  Listener *r{nullptr};
  if ("" == name || "default" == name) {
    auto net = get_net();
    if (net && net->is_service()) r = reinterpret_cast<Listener *>(net);
  } else {
    r = get_listener(name);
  }
  return r;
}

pf_db::Interface *Kernel::get_db(const std::string &name) {
  if (is_null(db_factory_) || db_list_.find(name) == db_list_.end()) 
    return nullptr;
  return db_factory_->getenv(db_list_[name]);
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
  if (!is_null(net_listener_factory_)) {
    for (auto it = listen_list_.begin(); it != listen_list_.end(); ++it) {
      auto net = net_listener_factory_->getenv(it->second);
      if (!is_null(net))
        this->newthread([&net]() { return thread::for_net(net); });
    }
  }
  if (!is_null(net_connector_))
    this->newthread(
        [this]() { return thread::for_net(net_connector_.get()); });
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

pf_net::connection::Basic *Kernel::default_connect(
    const std::string &name, const std::string &ip, uint16_t port) {
  using namespace pf_net::connection::manager;
  if (is_null(net_) || net_->is_service()) return nullptr;
  auto encrypt_str = GLOBALS["default.net.encrypt"].data;
  return connect(net_.get(), name, ip, port, encrypt_str);
}

pf_net::connection::Basic *Kernel::connect(const std::string &name) {
  if (connect_env_.find(name) == connect_env_.end()) return nullptr;
  auto id = connect_env_[name];
  auto ip = GLOBALS["client.ip" + std::to_string(id)].data;
  auto port = GLOBALS["client.port" + std::to_string(id)].get<uint16_t>();
  auto encrypt_str = GLOBALS["client.encrypt" + std::to_string(id)].data;
  auto connection = connect(name, ip, port, encrypt_str);
  if (!is_null(connection))
    connect_list_[name] = connection->get_id();
  return connection;
}

pf_net::connection::Basic *Kernel::connect(const std::string &name, 
                                           const std::string &ip, 
                                           uint16_t port, 
                                           const std::string &encrypt_str) {
  return connect(net_connector_.get(), name, ip, port, encrypt_str);
}

pf_net::connection::Basic *Kernel::connect(
    pf_net::connection::manager::Basic *client,
    const std::string &name, 
    const std::string &ip, 
    uint16_t port, 
    const std::string &encrypt_str) {
  using namespace pf_net::connection::manager;
  Connector *connector = dynamic_cast<Connector *>(client);
  if ("" == name) return nullptr;
  auto connection = connector->connect(ip.c_str(), port);
  if (is_null(connection)) return nullptr;
  //Register name.
  connection->set_name(name);
  pf_net::packet::RegisterConnectionName regname;
  regname.set_name(name);
  connection->send(&regname);
  net_connector_->set_connection_name(connection->get_id(), name);

  //Handshake.
  if (encrypt_str != "") {
    auto now = TIME_MANAGER_POINTER->get_ctime();
    char key[NET_PACKET_HANDSHAKE_KEY_SIZE]{0};
    std::string str{""};
    pf_basic::string::encrypt(encrypt_str, now, str);
    //std::cout << "str: " << str << std::endl;
    pf_basic::base64encode(key, str.c_str());
    //std::cout << "key: " << key << std::endl;
    pf_net::packet::Handshake handshake;
    handshake.set_key(key);
    connection->send(&handshake);
  }
  return connection;
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

  //Load the plugins.
  if (GLOBALS["plugins.count"] > 0) {
    auto count = GLOBALS["plugins.count"].get<uint8_t>();
    uint8_t i;
    for (i = 0; i < count; ++i) {
      std::string str = GLOBALS["plugins." + std::to_string(i)].data;
      if ("" == str) return false;
      std::vector<std::string> array;
      string::explode(str.c_str(), array, ":", true, true);
      type::variable_array_t params;
      bool seeglb{false};
      auto size = array.size();
      if (size > 1 && "global" == array[1]) seeglb = true;
      uint8_t j{2};
      for (; j < size; ++j)
        params.emplace_back(array[j]);
      if (!LIBRARY_MANAGER_POINTER->load(array[0], seeglb, params))
        return false;
    }
  }

  return true;
}

bool Kernel::init_net() {
  using namespace pf_net;
  using namespace pf_basic;
  SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                "[%s] Kernel::init_net start...", 
                ENGINE_MODULENAME);
  if (GLOBALS["default.net.open"] == true) {
    connection::manager::Basic *net{nullptr};
    auto conn_max = GLOBALS["default.net.conn_max"].get<uint16_t>();
    if (GLOBALS["default.net.service"] == true) {
      net = new connection::manager::Listener();
      unique_move(connection::manager::Basic, net, net_)
      auto service_ip = GLOBALS["default.net.service_ip"].c_str();
      auto service_port = GLOBALS["default.net.service_port"].get<uint16_t>();
      auto service = dynamic_cast< connection::manager::Listener *>(net);
      auto encrypt_str = GLOBALS["default.net.encrypt"].data;
      if (!service->init(conn_max, service_port, service_ip)) return false;
      std::string host{service->host()};
      if (encrypt_str != "") service->set_safe_encrypt_str(encrypt_str);
      SLOW_DEBUGLOG(ENGINE_MODULENAME,
                    "[%s] service listen at: host[%s] port[%d] max[%d].",
                    ENGINE_MODULENAME,
                    0 == host.size() ? "*" : host.c_str(),
                    service->port(),
                    conn_max);
    } else {
      net = new connection::manager::Connector();
      unique_move(connection::manager::Basic, net, net_)
      if (!net->init(conn_max)) return false;
    }
  }
  //Extra net listeners.
  if (GLOBALS["server.count"] > 0) {
    auto count = GLOBALS["server.count"].get<int8_t>();
    SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                  "[%s] Kernel::init_net extra service count: %d", 
                  ENGINE_MODULENAME,
                  count);
    using namespace pf_net::connection::manager;
    auto factory = new ListenerFactory();
    if (is_null(factory)) return false;
    unique_move(ListenerFactory, factory, net_listener_factory_);
    for (int8_t i = 0; i < count; ++i) {
      //From global values.
      auto name = GLOBALS["server.name" + std::to_string(i)].data;
      if ("" == name) {
        SLOW_ERRORLOG(ENGINE_MODULENAME,
                      "[%s] Kernel::init_net extra service can't get name: %d",
                      ENGINE_MODULENAME,
                      i);
        return false;
      }
      auto conn_max = 
        GLOBALS["server.connmax" + std::to_string(i)].get<uint16_t>();
      auto ip = GLOBALS["server.ip" + std::to_string(i)].data;
      auto port = GLOBALS["server.port" + std::to_string(i)].get<uint16_t>();
      auto encrypt_str = GLOBALS["server.encrypt" + std::to_string(i)].data;
      if (0 == port || conn_max <= 0) {
        SLOW_ERRORLOG(ENGINE_MODULENAME,
                      "[%s] Kernel::init_net extra service the port or "
                      "connection count error: [%d|%d|%d]",
                      ENGINE_MODULENAME,
                      port,
                      conn_max,
                      i);
        return false;
      }
      //Environment create.
      listener_config_t config;
      config.name = name;
      config.ip = ip;
      config.port = port;
      config.conn_max = conn_max;
      config.encrypt_str = encrypt_str;
      auto envid = net_listener_factory_->newenv(config);
      if (NET_EID_INVALID == envid) return false;
      listen_list_[name] = envid;
      listen_env_[name] = i;
      SLOW_DEBUGLOG(ENGINE_MODULENAME,
                    "[%s] service extra listen at: host[%s] port[%d] max[%d].",
                    ENGINE_MODULENAME,
                    0 == ip.size() ? "*" : ip.c_str(),
                    port,
                    conn_max);
    }
  }
  //Extra net connectors.
  if (GLOBALS["client.count"] > 0 || GLOBALS["client.usercount"] > 0) {
    using namespace pf_net::connection::manager;
    auto count = GLOBALS["client.count"].get<int8_t>();
    SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                    "[%s] Kernel::init_net extra client count: %d", 
                    ENGINE_MODULENAME,
                    count);
    auto connector = new Connector;
    if (is_null(connector)) return false;
    auto usercount = GLOBALS["client.usercount"].get<int8_t>();
    if (!connector->init(count + usercount + 1)) return false;
    auto reset_connect = [this](pf_net::connection::Basic *connection) {
      std::cout << "reset_connect" << std::endl;
      for (auto it = connect_list_.begin(); it != connect_list_.end(); ++it) {
        if (it->second == connection->get_id()) {
          std::cout << "reset: " << it->first << std::endl;
          it->second = -1; //Reset the hash to invalid.
          break;
        }
      }
    };
    if (count > 0) connector->callback_disconnect(reset_connect);
    unique_move(Connector, connector, net_connector_);
    for (int8_t i = 0; i < count; ++i) {
      auto name = GLOBALS["client.name" + std::to_string(i)].data;
      auto startup = GLOBALS["client.startup" + std::to_string(i)].get<bool>();
      if ("" == name) {
        SLOW_ERRORLOG(ENGINE_MODULENAME,
                    "[%s] Kernel::init_net extra client can't get name: %d",
                    ENGINE_MODULENAME,
                    i);
        return false;
      }
      connect_env_[name] = i;
      if (!startup) continue;
      //Try connect.
      connect(name);
    }
  }
  return true;
}

bool Kernel::init_db() {
  using namespace pf_db;
  register_env_creator_db(kDBEnvNull, db_null_env_creator);
  SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                "[%s] Kernel::init_db start...", 
                ENGINE_MODULENAME);
  if (GLOBALS["default.db.open"] == true || GLOBALS["database.count"] > 0) {
    auto factory = new Factory();
    if (is_null(factory)) return false;
    unique_move(Factory, factory, db_factory_);
  }
  if (GLOBALS["default.db.open"] == true) {
    config_t conf;
    conf.type = GLOBALS["default.db.type"].get<int8_t>();
    conf.name = GLOBALS["default.db.name"].c_str();
    conf.username = GLOBALS["default.db.user"].c_str();
    if (GLOBALS["default.db.encrypt"] == true) {
      std::string password{""};
      pf_basic::string::decrypt(GLOBALS["default.db.password"].data, password);
      conf.password = password;
    } else {
      conf.password = GLOBALS["default.db.password"].c_str();
    }
    db_eid_ = db_factory_->newenv(conf);
    if (DB_EID_INVALID == db_eid_) return false;
    auto env = db_factory_->getenv(db_eid_);
    if (!env->init()) return false;
  }
  
  //Extra.
  if (GLOBALS["database.count"] > 0) {
    auto count = GLOBALS["database.count"].get<int8_t>();
    SLOW_DEBUGLOG(ENGINE_MODULENAME, 
                  "[%s] Kernel::init_db the extra count: %d", 
                  ENGINE_MODULENAME,
                  count);
    for (int8_t i = 0; i < count; ++i) {
      config_t conf;
      auto name = GLOBALS["database.name" + std::to_string(i)];
      conf.type = GLOBALS["database.type" + std::to_string(i)].get<int8_t>();
      conf.name = GLOBALS["database.dbname" + std::to_string(i)].c_str();
      conf.username = GLOBALS["database.dbuser" + std::to_string(i)].c_str();
      if (GLOBALS["database.encrypt" + std::to_string(i)] == true) {
        std::string password{""};
        pf_basic::string::decrypt(
            GLOBALS["database.dbpassword" + std::to_string(i)].data, password);
        conf.password = password;
      } else {
        conf.password = 
          GLOBALS["database.dbpassword" + std::to_string(i)].c_str();
      }
      auto eid = db_factory_->newenv(conf);
      if (DB_EID_INVALID == eid) return false;
      auto env = db_factory_->getenv(eid);
      if (!env->init()) return false;
      db_list_[name] = eid;
    }
  }
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
  if (SCRIPT_EID_INVALID == script_eid_) {
    SLOW_ERRORLOG(ENGINE_MODULENAME,
                  "[%s] Kernel::init_script env(%d) create error",
                  ENGINE_MODULENAME,
                  GLOBALS["default.script.type"].get<int8_t>());
    return false;
  }
  auto env = script_factory_->getenv(script_eid_);
  if (!env->init()) {
    SLOW_ERRORLOG(ENGINE_MODULENAME,
                  "[%s] Kernel::init_script env->init() error",
                  ENGINE_MODULENAME);
    return false;
  }
  if (!env->bootstrap(GLOBALS["default.script.bootstrap"].c_str())) {
    SLOW_ERRORLOG(ENGINE_MODULENAME,
                  "[%s] Kernel::init_script env->bootstrap(%s) error",
                  ENGINE_MODULENAME,
                  GLOBALS["default.script.bootstrap"].c_str());
    return false;
  }
  return true;
}

//Get the net handle script function name.
const std::string Kernel::get_script_function(
    pf_net::connection::Basic *connection) {
  std::string key{""};
  auto listener = connection->get_listener();
  if (!is_null(listener) && listener->name() != "") {
    auto name = listener->name();
    auto configid = get_listen_configid(name);
    if (configid != -1)
      key = "server." + name + ".scriptfunc" + std::to_string(configid);
  } else if (connection->name() != "") {
    auto name = connection->name();
    auto configid = get_connect_configid(name);
    if (configid != -1)
      key = "client." + name + ".scriptfunc" + std::to_string(configid);
  }
  if (key == "" || GLOBALS[key] == "")
    key = "default.script.nethandle";
  std::string funcname = GLOBALS[key];
  return funcname;
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
    //Reconnect the connected.
    for (auto it = connect_list_.begin(); it != connect_list_.end(); ++it) {
      if (-1 == it->second) connect(it->first);
    }
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
