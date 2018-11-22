#include "app/config.h"

int32_t main(int32_t argc, char **argv) {

  using namespace pf_basic;

  pf_engine::Kernel engine;
  pf_engine::Application app(&engine, argc, argv);

  app.run();
  return 0;
}
