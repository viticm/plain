#include "plain/basic/global.h"

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

void InitGlobal::init() {
  auto&& g = get_globals();
  g.emplace("log.print", true);
}

InitGlobal g_init_global; // Auto init globals.

variable_map_t &get_globals() {
  static variable_map_t r;
  return r;
}

}
