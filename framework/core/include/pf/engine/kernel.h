/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id kernel.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/09 16:53
 * @uses the plian server engine kernel class
 */
#ifndef PF_ENGINE_KERNEL_H_
#define PF_ENGINE_KERNEL_H_

#include "pf/engine/config.h"
#include "pf/db/config.h"
#include "pf/script/config.h"
#include "pf/net/connection/manager/config.h"
#include "pf/net/protocol/config.h"
#include "pf/cache/manager.h"
#include "pf/basic/type/variable.h"
#include "pf/console/config.h"

namespace pf_engine {

class PF_API Kernel : public pf_basic::Singleton< Kernel > {

 public:
   Kernel();
   virtual ~Kernel();

 public:
   static Kernel &getsingleton();
   static Kernel *getsingleton_pointer();

 public:
   virtual bool init();
   virtual void run();
   virtual void stop();

 public:
   pf_net::connection::manager::Basic *get_net() {
     return net_.get();
   };
   pf_db::Interface *get_db();
   pf_cache::Manager *get_cache() {
     return cache_.get();
   };
   pf_script::Interface *get_script();
   pf_db::Factory *get_db_factory() { return db_factory_.get(); }
   pf_script::Factory *get_script_factory() { return script_factory_.get(); }
   pf_console::Application *get_console() { return console_.get(); }

 public:

   //Use these interface will send handshake packt after connected.
   pf_net::connection::Basic *default_connect(const std::string &name, 
                                              const std::string &ip, 
                                              uint16_t port, 
                                              const std::string &encrypt_str = "");
   pf_net::connection::Basic *connect(const std::string &name);
   pf_net::connection::Basic *connect(const std::string &name, 
                                      const std::string &ip, 
                                      uint16_t port, 
                                      const std::string &encrypt_str = "");
   pf_net::connection::Basic *connect(pf_net::connection::manager::Basic *,
                                      const std::string &name, 
                                      const std::string &ip, 
                                      uint16_t port, 
                                      const std::string &encrypt_str = "");

   //Get the extra listener or connector.
   pf_net::connection::Basic *get_connector(const std::string &name);
   pf_net::connection::manager::Listener *get_listener(const std::string &name);
   pf_net::connection::manager::Connector *get_connector() {
     return net_connector_.get();
   };
   pf_db::Interface *get_db(const std::string &name);

   //Get the service from name(default or listen list).
   pf_net::connection::manager::Listener *get_service(const std::string &name);

   //Get config id.
   int8_t get_listen_configid(const std::string &name) {
     if (listen_env_.find(name) == listen_env_.end()) return -1;
     return listen_env_[name];
   }

   std::map<std::string, int8_t> get_listen_list() const {
     return listen_list_;
   }

   int8_t get_connect_configid(const std::string &name) {
     if (connect_env_.find(name) == connect_env_.end()) return -1;
     return connect_env_[name];
   }

   std::map<std::string, int8_t> get_connect_list() const {
     return connect_list_;
   }
   
   std::map<std::string, int8_t> get_db_list() const {
     return db_list_;
   }

   // Get the net handle script function name.
   const std::string get_script_function(pf_net::connection::Basic *);
   
   // Get net listener factory.
   pf_net::connection::manager::ListenerFactory *get_net_listener_factory(bool);

 public:
   //Enqueue an envet function in main loop.
   template<class F, class... Args>
   auto enqueue(F&& f, Args&&... args) 
   -> std::future<typename std::result_of<F(Args...)>::type>;

 public:
   template<class F, class... Args>
   std::thread::id newthread(const std::string &name, F&& f, Args&&... args);

 protected:
   virtual bool init_base();
   virtual bool init_net();
   virtual bool init_db();
   virtual bool init_cache();
   virtual bool init_script();
   virtual bool init_console();

 protected:
   std::unique_ptr<pf_net::connection::manager::Basic> net_;
   std::unique_ptr<pf_db::Factory> db_factory_;
   std::unique_ptr<
     pf_net::connection::manager::ListenerFactory> net_listener_factory_;
   std::unique_ptr<pf_net::connection::manager::Connector> net_connector_;
   pf_db::eid_t db_eid_;
   std::unique_ptr<pf_cache::Manager> cache_;
   std::unique_ptr<pf_script::Factory> script_factory_;
   std::unique_ptr<pf_console::Application> console_;
   pf_script::eid_t script_eid_;
   std::vector< std::thread > thread_workers_;
   std::map<std::string, int8_t> db_list_;  //Database name to factory id.
   std::map<std::string, int8_t> listen_list_; //Listen net name to factory id.
   std::map<std::string, int8_t> connect_list_; //Connect net name to id.
   std::map<std::string, int8_t> connect_env_; //Connect net name to config id.
   std::map<std::string, int8_t> listen_env_; //Listen net name to config id.
   bool isinit_;

 private:
   void loop();

 private:
   std::queue< std::function<void()> > tasks_;
   std::vector< std::function<void()> > thread_tasks_;
   std::mutex queue_mutex_;
   bool stop_;

};

} //namespace pf_engine

#include "pf/engine/kernel.tcc"

PF_API extern std::unique_ptr< pf_engine::Kernel > g_engine;
#define ENGINE_POINTER pf_engine::Kernel::getsingleton_pointer()

#endif //PF_ENGINE_KERNEL_H_
