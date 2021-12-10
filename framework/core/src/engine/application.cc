#include "pf/sys/process.h" 
#include "pf/basic/string.h"
#include "pf/basic/time_manager.h"
#include "pf/basic/util.h"
#include "pf/basic/io.tcc"
#include "pf/sys/util.h"
#include "pf/file/ini.h"
#include "pf/file/api.h"
#include "pf/script/interface.h"
#include "pf/engine/kernel.h"
#include "pf/engine/application.h"

using namespace pf_engine;

/* bits of various argument indicators in 'args' */
#define has_error 1 /* bad option */
#define has_d 2 /* -d or --daemon */
#define has_v 4 /* -v or --version */
#define has_h 8 /* -h or --help */

static bool pidfile_isexists(bool perr) {
  using namespace pf_basic;
  if (pf_file::api::exists(GLOBALS["app.pidfile"].data)) {
    if (perr) io_cerr("The application process id file has exists");
    return true;
  }
  return false;
}

/* all default functions { */
void daemon() {
  if (pidfile_isexists(true)) return;
  pf_sys::process::daemon();
  Application::getsingleton().without_command_run();
}

void reload() {
  auto script = ENGINE_POINTER->get_script();
  if (!is_null(script))
    script->reload(GLOBALS["default.script.reload"].c_str());
}

void _stop() {
  if (GLOBALS["app.pidfile"] != "") {
    pf_sys::process::waitexit(GLOBALS["app.pidfile"].c_str());
    remove(GLOBALS["app.pidfile"].c_str());
  }
}

void helps() {
  Application::getsingleton().view_commands();
}

void version() {
	printf("The plain frame version: %s\n", PF_COPYRIGHT);
}

#if OS_UNIX /* { */
void signal_handler(int32_t signal) {
  using namespace pf_basic;
  if (GLOBALS["app.status"] == kAppStatusStop) return;
  //处理前台模式信号
  static uint32_t last_signaltime = 0;
  uint32_t currenttime = TIME_MANAGER_POINTER->get_tickcount();
  if (signal == SIGINT) {
    if (currenttime - last_signaltime > 10 * 1000) {
      io_cdebug(
          "\r[%s] (signal_handler) got SIGINT[%d] engine will reload!", 
          GLOBALS["app.name"].c_str(),
          signal);
      Application::getsingleton().parse_command("reload");
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
  if (GLOBALS["app.status"] == kAppStatusStop) return;
  using namespace pf_basic;
  static uint32_t last_signaltime = 0;
  uint32_t currenttime = TIME_MANAGER_POINTER->get_tickcount();
  switch (event) {
    case CTRL_C_EVENT: {
      if (currenttime - last_signaltime > 10 * 1000) {
        io_cdebug(
            "[%s] (signal_handler) CTRL+C received, engine will reload!",
            GLOBALS["app.name"].c_str());
        Application::getsingleton().parse_command("reload");
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

Application::Application(Kernel *engine, int32_t argc, char *argv[]) {
  UNUSED(argc);
  using namespace pf_basic::util;
  using namespace pf_basic::string;
#if OS_UNIX
  register_commandhandler("daemon", "run application with daemon(-d)", daemon);
  register_commandhandler("stop", "stop the application", _stop);
#endif
  register_commandhandler("help", "view help text(-h)", helps);
  register_commandhandler("version", "view plain framework version(-v)", version);
  register_commandhandler("reload", "reload script files", reload);
  engine_ = engine;
  args_flag_ = 0;

  //Parser args.
  int32_t i;
  std::vector<std::string> array;
  for (i = 1; argv[i] != NULL; ++i) {
    if (argv[i][0] != '-')
      break;
    switch (argv[i][1]) {  /* else check option */
      case '-':  /* '--' */ 
        if (argv[i][2] == '\0' || argv[i][3] == '\0') {
          args_flag_ = has_error;
          break;
        }
        array.clear();
        explode(argv[i] + 2, array, "=", true, true);
        if (array.size() > 2) {
          args_flag_ = has_error;
          break;
        }
        if ("daemon" == array[0]) {
          args_flag_ |= has_d;
        } else if ("help" == array[0]) {
          args_flag_ |= has_h;
          break;
        } else if ("version" == array[0]) {
          args_flag_ |= has_v;
          break;
        } else {
          if (2 == array.size())
            args_[array[0]] = array[1];
          else
            args_[array[0]] = "1";
        }
        break;
      case '\0':
        break;
      case 'd':
        if (argv[i][2] != '\0') {
          args_flag_ = has_error;
          break;
        }
        args_flag_ |= has_d;
        break;
      case 'v':
        if (argv[i][2] != '\0') {
          args_flag_ = has_error;
          break;
        }
        args_flag_ |= has_v;
        break;
      case 'h':
        if (argv[i][2] != '\0') {
          args_flag_ = has_error;
          break;
        }
        args_flag_ |= has_h;
        break;
      default:
        args_flag_ = has_error;
        break;
    }
  }
}

Application::~Application() {
  engine_ = nullptr;
}   

//Set the framework global values by env file.
bool Application::set_env_globals(bool perr) {
  using namespace pf_basic;
  pf_file::Ini _env;
  auto env_file = get_arg("env");
  if (env_file == "" || !_env.open(env_file.c_str())) {
    if (perr) io_cerr("Can't load the env file!");
    return false;
  }

  auto sectiondata = _env.getdata();
  for (auto find_it = sectiondata->begin(); 
       find_it != sectiondata->end(); 
       ++find_it) {
    auto sets = find_it->second;
    for (auto it = sets->begin(); it != sets->end(); ++it) {
      auto section = find_it->first;
      auto key = it->first;
      std::string name = section + "." + key;
      pf_basic::type::variable_t value;
      _env.get(section.c_str(), key.c_str(), value);
      /**
      printf("section: %s key: %s, value: %s\n", 
              section.c_str(), key.c_str(), value.c_str());
      **/
      GLOBALS[name] = value;
    }
  }
  auto frame = GLOBALS["default.engine.frame"].get<uint16_t>();
  if (0 < frame && frame < 1000) {
    uint16_t frame_time = 1000 / frame;
    GLOBALS["default.engine.frame_time"] = frame_time;
  } else {
    io_cwarn(
        "[%s] (set_env_globals) frame(%d) is invalid",
        GLOBALS["app.name"].c_str(),
        frame);
  }
  return true;
}

bool Application::init() {
  return true;
}

bool Application::without_command_run() {
  if (!init()) return false;
  start();
  return true;
}

void Application::set_pidfile() {
  using namespace pf_basic;
  if (args_["pidfile"] != "") {
    GLOBALS["app.pidfile"] = args_["pidfile"];
  } else if (GLOBALS["app.pidfile"] == "") {
    char process_idpath[FILENAME_MAX] = {0};
   if (GLOBALS["app.name"] != "") {
    snprintf(process_idpath, 
             sizeof(process_idpath) - 1, 
             "%s%s.pid", 
             GLOBALS["app.basepath"].c_str(), 
             GLOBALS["app.name"].c_str());
    } else {
      pf_sys::process::get_filename(process_idpath, sizeof(process_idpath));
    }
    GLOBALS["app.pidfile"] = process_idpath;
  }
}

void Application::run() {
  if (args_flag_ & has_error) {
    pf_basic::io_cerr("The application args error!");
  } else if ("1" == args_["stop"]) {
    set_env_globals(); set_pidfile(); //Use pid file.
    parse_command("stop");
  } else if (args_flag_ & has_v) {
    parse_command("version");
  } else if (args_flag_ & has_h) {
    parse_command("help");
  } else if (args_flag_ & has_d) {
    set_env_globals(); set_pidfile(); //Use pid file.
    parse_command("daemon");
  } else {
    if (!set_env_globals(true)) return; 
    set_pidfile();
    if (pidfile_isexists(true)) return;
    without_command_run();
  }
}

void Application::start() {
  using namespace pf_basic;  
  using namespace pf_basic::util;

#if OS_WIN 
  if (GLOBALS["app.console"] == true) {
    _CrtSetDbgFlag(_CrtSetDbgFlag(0) | _CRTDBG_LEAK_CHECK_DF);
    system("color 02"); //color green
    system("mode con cols=120"); //cmd size
  }
#endif

  //Environment.
#if OS_UNIX
  if (GLOBALS["app.debug"] == 1 && !(args_flag_ & has_d)) {
    char filename[FILENAME_MAX] = {0};
    get_module_filename(filename, sizeof(filename) - 1);
    io_cerr("----------------------------------------"
            "----------------------------------------");
    io_cerr("                                   [WARNING]");
    io_cerr(" app not run in daemon mode, "
                "it will be auto stop when user logout or session");
    io_cerr(" disconnect. should add daemon option to start server"
                " as daemon mode, such as:");
    io_cdebug("     %s --daemon(or -d)", filename);
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
  if (!pf_sys::process::writeid(GLOBALS["app.pidfile"].c_str())) { 
    io_cerr("[%s] process id file: %s write error", 
            GLOBALS["app.name"].c_str(), 
            GLOBALS["app.pidfile"].c_str());
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

  //Not wait the unhandle threads(debug).
  if (GLOBALS["app.forceexit"] == true) exit(0);
}
   
void Application::stop() {
  engine_->stop();
  remove(GLOBALS["app.pidfile"].c_str());
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
    printf("\t--%s: %s\n", it->first.c_str(), it->second.c_str());
  }
}
