#include "pf/basic/string.h"
#include "pf/support/helpers.h"
#include "pf/console/parser.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_console;

std::tuple< std::string, std::vector<InputArgument>, std::vector<InputOption> > 
Parser::parse(const std::string &expression) {
  auto _name = name(expression);
  std::vector<std::string> r;
  std::regex reg("\\{\\s*(.*?)\\s*\\}");
  std::smatch match;
  std::vector<InputArgument> arguments;
  std::vector<InputOption> options;

  if (std::regex_match(expression, match, reg)) {
    auto temp = parameters({match.str(1),});
    std::tie (arguments, options) = temp;
  }
  return std::make_tuple(_name, arguments, options);
}

std::string Parser::name(const std::string &expression) {
  std::smatch match;
  std::regex reg("[^\\s]+");
  if (std::regex_match(expression, match, reg)) {
    return match.str();
  }
  return "";
}

std::tuple< std::vector<InputArgument>, std::vector<InputOption> >
Parser::parameters(const std::vector<std::string> &tokens) {
  std::vector<InputArgument> arguments;
  std::vector<InputOption> options;
  std::regex reg("-{2,}(.*)");
  std::smatch match;
  for (const auto &token : tokens) {
    if (std::regex_match(token, match, reg)) {
      options.emplace_back(parse_option(match.str(1)));
    } else {
      arguments.emplace_back(parse_argument(token));
    }
  }
  return std::make_tuple(arguments, options);
}

InputArgument Parser::parse_argument(const std::string &_token) {
  auto temp = extract_description(_token);
  auto token = temp[0]; auto description = temp[1];
  auto size = token.size();
  /**
  std::cout << (token.substr(size - 1, 2) == "?*") << std::endl;
  switch (true) {
    case (token.substr(size - 1, 2) == "?*"):
      return InputArgument(
          trim(token, "?*"), InputArgument::kModeIsArray, description);

  }
  **/
  std::smatch match;
  if (token.substr(size - 2, 2) == "?*") {
    return InputArgument(
        trim(token, "?*"), InputArgument::kModeIsArray, description, "");
  } else if (token[size - 1] == '*') {
    return InputArgument(
        trim(token, "*"), 
        InputArgument::kModeIsArray | InputArgument::kModeRequired, 
        description, "");
  } else if (token[size - 1] == '?') {
    return InputArgument(
        trim(token, "?"), 
        InputArgument::kModeOptional, 
        description, "");
  } else if (std::regex_match(token, match, std::regex("(.+)\\=\\*(.+)"))) {
    std::string def_str = match.str(2);
    std::regex reg(",\\s?");
    std::sregex_token_iterator it(def_str.begin(), def_str.end(), reg, -1);
    decltype(it) it_end;
    std::vector<std::string> defs;
    for (; it != it_end; ++it)
      defs.emplace_back(it->str());
    return InputArgument(
        match.str(1), InputArgument::kModeIsArray, description, defs);
  } else if (std::regex_match(token, match, std::regex("(.+)\\=(.+)"))) {
  return InputArgument(
      match.str(1), InputArgument::kModeOptional, description, match.str(2));
  }
  return InputArgument(token, InputArgument::kModeRequired, description, "");
}

InputOption Parser::parse_option(const std::string &_token) {
  auto temp = extract_description(_token);
  auto token = temp[0]; auto description = temp[1];
  auto size = token.size();
  std::string shortcut{""};
  std::regex reg("\\s*\\|\\s*");
  std::sregex_token_iterator it(token.begin(), token.end(), reg, 2);
  decltype(it) it_end;
  auto temp1 = it->str();
  std::smatch match;
  if (++it != it_end) {
    shortcut = temp1;
    token = it->str();
  }
  if (token.substr(size - 2, 2) == "=*") {
    return InputOption(
        trim(token, "=*"), 
        shortcut,
        InputOption::kModeIsArray | InputOption::kModeOptional, 
        description, "");
  } else if (token[size - 1] == '=') {
    return InputOption(
        trim(token, "="), 
        shortcut,
        InputArgument::kModeOptional, 
        description, "");
  } else if (std::regex_match(token, match, std::regex("(.+)\\=\\*(.+)"))) {
    std::string def_str = match.str(2);
    std::regex reg1(",\\s?");
    std::sregex_token_iterator it1(def_str.begin(), def_str.end(), reg1, -1);
    decltype(it1) it_end1;
    std::vector<std::string> defs;
    for (; it1 != it_end1; ++it1)
      defs.emplace_back(it1->str());
    return InputOption(
        match.str(1), 
        shortcut,
        InputOption::kModeIsArray | InputOption::kModeOptional, 
        description, defs);
  } else if (std::regex_match(token, match, std::regex("(.+)\\=(.+)"))) {
    return InputOption(
      match.str(1), shortcut, 
      InputOption::kModeOptional, description, match.str(2));
  }

  return InputOption(token, shortcut, InputOption::kModeNone, description, "");
}

std::vector<std::string> Parser::extract_description(const std::string &token) {
  std::vector<std::string> r;
  std::regex reg("\\s+:\\s+");
  std::sregex_token_iterator it(token.begin(), token.end(), reg, -1);
  decltype(it) it_end;
  size_t c{0};
  bool find{true};
  for (; it != it_end; ++it) {
    r.emplace_back(it->str());
    ++c;
    if (c >= 2) {
      find = false;
      break;
    }
  }
  if (!find) {
    r.clear();
    r.emplace_back(token);
    r.emplace_back("");
  }
  return r;
}
