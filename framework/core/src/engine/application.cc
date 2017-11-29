#include "pf/sys/process.h" 
#include "pf/basic/time_manager.h"
#include "pf/basic/util.h"
#include "pf/basic/io.tcc"
#include "pf/sys/util.h"
#include "pf/engine/kernel.h"
#include "pf/engine/application.h"

using namespace pf_engine;

/* all default functions { */
void daemon() {
  pf_sys::process::daemon();
  Application::getsingleton().run();
}

void _stop() {
  char process_idpath[FILENAME_MAX] = {0};
  pf_sys::process::get_filename(process_idpath, sizeof(process_idpath));
  pf_sys::process::waitexit(process_idpath);
}

void helps() {
  Application::getsingleton().view_commands();
}

#if OS_UNIX /* { */
void signal_handler(int32_t signal) {
  using namespace pf_basic;
  //处理前台模式信号
  static uint32_t last_signaltime = 0;
  uint32_t currenttime = TIME_MANAGER_POINTER->get_tickcount();
  if (signal == SIGINT) {
    if (currenttime - last_signaltime > 10 * 1000) {
      io_cdebug(
          "\r[%s] (signal_handler) got SIGINT[%d] engine will reload!", 
          GLOBALS["app.name"].c_str(),
          signal);
      Application::getsingleton().parse_command("--reload");
    } else {
      io_cwarn(
          "\r[%s] (signal_handler) got SIGINT[%d] engine will stop!", 
          GLOBALS["app.name"].c_str(),
          signal);
      Application::getsingleton().stop();
    }
  }
  //处理后台模式信号
  if (signal == SIGUSR1) {
    io_cwarn(
        "\r[%s] (signal_handler) got SIGUSR1[%d] engine will stop!", 
        GLOBALS["app.name"].c_str(),
        signal);
    Application::getsingleton().stop();
  }
  last_signaltime = currenttime;
}
#elif OS_WIN /* }{ */
BOOL WINAPI signal_handler(DWORD event) {
  using namespace pf_basic;
  static uint32_t last_signaltime = 0;
  uint32_t currenttime = TIME_MANAGER_POINTER->get_tickcount();
  switch (event) {
    case CTRL_C_EVENT: {
      if (currenttime - last_signaltime > 10 * 1000) {
        io_cdebug(
            "[%s] (signal_handler) CTRL+C received, engine will reload!",
            GLOBALS["app.name"].c_str());
        Application::getsingleton().parse_command("--reload");
      } else {
        io_cwarn(
            "[%s] (signal_handler) CTRL+C received, engine will stop!",
            GLOBALS["app.name"].c_str());
        Application::getsingleton().stop();
      }
      break;
    }
    default:
      break;
  }
  last_signaltime = currenttime;
  return TRUE;
}
#endif
/* } all default functions */

template <>
Application * pf_basic::Singleton< Application >::singleton_ = nullptr;

Application *Application::getsingleton_pointer() {
  return singleton_;
}

Application &Application::getsingleton() {
  Assert(singleton_);
  return *singleton_;
}

Application::Application(Kernel *engine) {
  using namespace pf_basic::util;
#if OS_UNIX
  register_commandhandler("--daemon", "run application with daemon", daemon);
  register_commandhandler("--stop", "stop the application", _stop);
  register_commandhandler("--help", "view help text", helps);
#endif
  engine_ = engine;
}

Application::~Application() {
  engine_ = nullptr;
}   

void Application::run() {
  using namespace pf_basic;  
  using namespace pf_basic::util;
  char process_idpath[FILENAME_MAX] = {0};
  pf_sys::process::get_filename(process_idpath, sizeof(process_idpath));
#if OS_WIN 
  if (GLOBALS["app.console"] == true) {
    _CrtSetDbgFlag(_CrtSetDbgFlag(0) | _CRTDBG_LEAK_CHECK_DF);
    system("color 02"); //color green
    system("mode con cols=120"); //cmd size
  }
#endif

  //Environment.
#if OS_UNIX
  if (GLOBALS["app.debug"] == 1) {
    char filename[FILENAME_MAX] = {0};
    get_module_filename(filename, sizeof(filename) - 1);
    io_cerr("----------------------------------------"
            "----------------------------------------");
    io_cerr("                                   [WARNING]");
    io_cerr(" app not run in daemon mode, "
                "it will be auto stop when user logout or session");
    io_cerr(" disconnect. should add daemon option to start server"
                " as daemon mode, such as:");
    io_cdebug("     %s --daemon", filename);
    io_cerr("----------------------------------------"
            "----------------------------------------");
  }
  signal(SIGPIPE, SIG_IGN); //Socket has error will get this signal.
  if (!pf_sys::util::set_core_rlimit()) {
    io_cerr("[app] (Application::run) change core rlimit failed!");
    return;
  }
#endif
  
#if OS_WIN
  WORD versionrequested;
  WSADATA data;
  int32_t error;
  versionrequested = MAKEWORD(2, 2);
  error = WSAStartup(versionrequested, &data);
#endif
  if (nullptr == engine_) return;
  if (!engine_->init()) return;
  if (!pf_sys::process::writeid(process_idpath)) { 
    io_cerr("[%s] process id file: %s write error", 
            GLOBALS["app.name"].c_str(), 
            process_idpath);
    return;
  }
#if OS_UNIX
  signal(SIGINT, signal_handler);
  signal(SIGUSR1, signal_handler);
#elif OS_WIN 
  pf_basic::util::disable_windowclose();
  if (SetConsoleCtrlHandler(
        (PHANDLER_ROUTINE)signal_handler, TRUE) != TRUE) {
    io_cerr("[%s] can't install signal handler", GLOBALS["app.name"].c_str());
    return;
  }
#endif
  engine_->run();
}
   
void Application::run(int32_t argc, char *argv[]) {
  if (argc > 1) {
    parse_command(argv[1]);
  } else {
    run();
  }
  //Not wait the unhandle threads(debug).
  if (GLOBALS["app.forceexit"] == true) exit(0);
}

void Application::stop() {
  char process_idpath[FILENAME_MAX] = {0};
  pf_sys::process::get_filename(process_idpath, sizeof(process_idpath));
  engine_->stop();
  remove(process_idpath);
}

void Application::register_commandhandler(
    const char *command, 
    const char *description, 
    function_commandhandler commandhandler) {
  if (nullptr == command || 0 == strcmp(command, "")) return;
  command_functions_[command] = commandhandler;
  command_descriptions_[command] = nullptr == description ? "" : description;
}

void Application::parse_command(const char *command) {
  function_commandhandler commandhandler = command_functions_[command];
  if (commandhandler) (*commandhandler)();
}

bool Application::env() {
  using namespace pf_basic;
  if (is_null(TIME_MANAGER_POINTER)) {
    auto time_manager = new TimeManager();
    if (is_null(time_manager)) return false;
    unique_move(TimeManager, time_manager, g_time_manager);
    if (!g_time_manager->init()) return false;
  }

  if (is_null(LOGSYSTEM_POINTER)) {
    auto logger = new Logger();
    if (is_null(logger)) return false;
    unique_move(Logger, logger, g_logger);
  }
  return true;
}
   
void Application::view_commands() {
  using namespace pf_basic;
  io_cdebug("Long options:");
  std::map< std::string, std::string >::iterator it;
  for (it = command_descriptions_.begin(); 
       it != command_descriptions_.end();
       ++it) {
    printf("\t%s %s\n", it->first.c_str(), it->second.c_str());
  }
}
