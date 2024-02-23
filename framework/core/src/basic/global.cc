#include "plain/basic/global.h"
#include <filesystem>
#include "plain/basic/utility.h"
#include "plain/engine/config.h"

namespace plain {

class InitGlobal {

 public:
  InitGlobal() {
    init();
  }
  ~InitGlobal() = default;

 private:
  void init();

};

#define SET(key,value) g.emplace(key, value)

void InitGlobal::init() {
  auto&& g = get_globals();

  // App settings.
  SET("app.name", "unknown");
  SET("app.debug", false);
  SET("app.status", AppStatus::Stopped);
  auto current_directory{std::filesystem::current_path()};
  auto path = current_directory.string();
#if OS_WIN
  path_tounix(path.c_str(), path.size());
#endif

  SET("app.basepath", path);
  
  // Logs settings.
  SET("log.print", false);
  auto log_directory = path + "/log";
  SET("log.directory", log_directory);
}

InitGlobal g_init_global; // Auto init globals.

variable_map_t &get_globals() {
  static variable_map_t r;
  return r;
}

}
