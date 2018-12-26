/**
 * PAP Engine ( https://github.com/viticm/plainframework1 )
 * $Id application.h
 * @link https://github.com/viticm/plainframework1 for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm@126.com/viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm@126.com/viticm.ti@gmail.com>
 * @date 2015/08/23 17:59
 * @uses The plain framework engine for application.
 */
#ifndef PF_ENGINE_APPLICATION_H_
#define PF_ENGINE_APPLICATION_H_

#include "pf/engine/config.h"
#include "pf/basic/singleton.tcc"

namespace pf_engine {

typedef void (__stdcall *function_commandhandler)();

class PF_API Application : public pf_basic::Singleton<Application> {

 public:
   Application(Kernel *engine, int32_t argc, char *argv[]);
   ~Application();

 public:
   static Application &getsingleton();
   static Application *getsingleton_pointer();

 public:
   void run();
   void stop();
   bool without_command_run();

 public:
   static bool env(); /* Basic environment. */

 public:
   void register_commandhandler(const char *command, 
                                const char *description, 
                                function_commandhandler commandhandler);
   void parse_command(const char *command);
   void view_commands();
   const std::string get_arg(const std::string &name) {
     return args_[name];
   };

 private:
   bool init();
   void start();
   bool set_env_globals(bool perr = false);
   void set_pidfile();

 private:
   std::map< std::string, function_commandhandler > command_functions_;
   std::map< std::string, std::string > command_descriptions_;
   std::map< std::string, std::string > args_;
   int32_t args_flag_;
   Kernel *engine_;

};

} //namespace pf_engine

#endif //PF_ENGINE_APPLICATION_H_
