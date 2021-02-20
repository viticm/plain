#include <regex>
#include "pf/basic/string.h"
#include "pf/console/string_input.h"

using namespace pf_basic::string;
using namespace pf_console;

const std::string kRegexString("([^\\s]+?)(?:\\s|(?<!\\\\)\"|(?<!\\\\)'|$)\"))");
const std::string kRegexQuotedString(
    "(?:\"([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)\"|'"
    "([^'\\\\]*(?:\\\\.[^'\\\\]*)*)')])])])\"])\")");

std::vector<std::string>
StringInput::tokenize(const std::string &input) const {
  std::vector<std::string> tokens;
  auto length = input.size();
  decltype(length) cursor{0};
  auto begin = input.cbegin();
  auto end = input.cend();
  while (cursor < length) {
    std::smatch match;
    auto cur = begin + cursor;
    std::string token{""};

    if (std::regex_search(cur, end, match, std::regex("^\\s+"))) {
      //Jump the spaces.
    } else if (std::regex_search(cur, end, match, 
			std::regex("^([^=\"'\\s]+?)(=?)(" + kRegexQuotedString + "+)"))) {
      auto temp = str_replace("\\", "", 
          str_replaces({"\"'", "'\"", "''", "\"\""}, 
            "", match.str(3).substr(1, match.str(3).size() - 2)));
      token = match.str(1) + match.str(2) + temp;
    } else if (std::regex_search(
          cur, end, match, std::regex("^" + kRegexQuotedString))) {
      token = str_replace(
          "\\", "", match.str().substr(1, match.str().size() - 2));
    } else if (std::regex_search(
          cur, end, match, std::regex("^" + kRegexString))) {
      token = str_replace("\\", "", match.str());
    } else {
      std::string e;
      e = "Unable to parse input near \"... " 
        + input.substr(cursor, 10) + " ... \"";
      throw e;
    }
    if (token != "") {
      tokens.emplace_back(token);
    }
    cursor += match[0].str().size();
  }
  return tokens;
}
