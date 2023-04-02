#include "plain/basic/global.h"

namespace plain {

variable_map_t &get_globals() {
  static variable_map_t r;
  return r;
}

}
