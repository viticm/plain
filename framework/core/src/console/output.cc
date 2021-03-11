#include "pf/basic/global.h"
#include "pf/basic/logger.h"
#include "pf/console/output.h"

using namespace pf_console;

void Output::write(
    const std::string &messages, bool newline, uint16_t options) {
  char filename[FILENAME_MAX]{0};
  pf_basic::Logger::get_log_filename("console", filename);
  FILE *fp;
  fp = fopen(filename, "ab");
  if (is_null(fp)) return;
  fwrite(messages.c_str(), 1, messages.size(), fp);
  std::cout << messages ;
  if (newline) {
    fwrite(LF, 1, 4, fp);
    std::cout << std::endl;
  }
  fclose(fp);
}
