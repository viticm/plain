#include "pf/all.h"

int32_t main(int32_t argc, char **argv) {
  pf_engine::Kernel engine;
  pf_engine::Application app(&engine);
  app.run(argc, argv);
  std::cout << "main" << std::endl;
  return 0;
}
