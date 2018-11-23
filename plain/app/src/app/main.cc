#include "app/config.h"

int32_t main(int32_t argc, char **argv) {

  using namespace pf_basic;

  pf_engine::Kernel engine;
  pf_engine::Application app(&engine, argc, argv);

  std::string str{"fjkcdddfesfasdfasdfasdfasdfasdfasdellllljyuii"}; int32_t time = 2333;
  std::string en_str{""};
  std::string de_str{""};
  int32_t de_time{0};
  string::encrypt(str, time, en_str);
  string::decrypt(en_str, de_time, de_str);

  std::cout << "en_str: " << en_str << " de_str: " 
    << de_str << " de_time: " << de_time << std::endl;

  app.run();
  return 0;
}
