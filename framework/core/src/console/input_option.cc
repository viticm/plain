#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/console/input_option.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_console;

void InputOption::set_shortcut(const std::string &_shortcut) {
  // std::cout << "_shortcut: " << _shortcut << std::endl;
  std::string shortcut{_shortcut};
  ltrim(shortcut, "-");
  std::regex e("(\\|)-?");
  std::sregex_token_iterator pos(shortcut.begin(), shortcut.end(), e, -1);
  decltype(pos) pos_end;
  std::vector<std::string> shortcuts;
  for (; pos != pos_end; ++pos) {
    shortcuts.emplace_back(pos->str());
  }
  shortcut_ = implode("|", shortcuts);
}
