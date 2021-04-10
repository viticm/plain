#include "pf/basic/global.h"
#include "pf/support/helpers.h"
#include "pf/basic/string.h"
#include "pf/console/argv_input.h"

using namespace pf_support;
using namespace pf_console;
using namespace pf_basic;

ArgvInput::ArgvInput( 
    const std::vector<std::string> &argv, 
    InputDefinition *definition, bool no_app_name) 
  : Input(definition) {
  std::vector<std::string> _argv = argv;
  if (_argv.empty() && !empty(GLOBALS["argv"])) {
    auto temp = explode(" ", GLOBALS["argv"].c_str());
    for (const auto & el: temp) {
      _argv.emplace_back(el.data);
    }
  }
  if (!no_app_name && !_argv.empty()) { // Remove application name.
    _argv.erase(_argv.begin());
  }
  tokens_ = _argv;
}

void ArgvInput::parse() {
  bool parse_options{true};
  for (int32_t i = tokens_.size() - 1; i >= 0; --i) {
    parsed_.emplace_back(tokens_[i]);
  }
  while (!parsed_.empty()) {
    auto token = parsed_.back(); parsed_.pop_back();
    // std::cout << "token: " << token << std::endl;
    if (parse_options && "" == token) {
      parse_argument(token);
    } else if (parse_options && "--" == token) {
      parse_options = false;
    } else if (parse_options && "--" == token.substr(0, 2)) {
      parse_long_option(token);
    } else if (parse_options && '-' == token[0] && "-" != token) {
      parse_short_option(token);
    } else {
      parse_argument(token);
    }
  }
  /**
  for (auto &op : options_) {
    std::cout << "op: " << op.first << " v: " << op.second << std::endl;
  }
  **/
}

void ArgvInput::parse_short_option(const std::string &token) {
  auto name = token.substr(1);
  if (name.size() > 1) {
    auto shortcut = name.substr(0, 1);
    if (definition_->has_shortcut(shortcut) &&
        definition_->get_option_for_shortcut(shortcut).accept_value()) {
      // an option with a value (with no space)
      add_short_option(shortcut, name.substr(1));
    } else {
      parse_short_option_set(name);
    }
  } else {
    add_short_option(name, "");
  }
}

void ArgvInput::parse_short_option_set(const std::string &name) {
  auto length = name.size();
  for (decltype(length) i = 0; i < length; ++i) {
    auto shortcut = name.substr(i, 1);
    if (!definition_->has_shortcut(shortcut)) {
      std::string e{""};
      e += "The \"-\"" + shortcut + " option does not exist.";
      throw std::runtime_error(e);
    }
    auto option = definition_->get_option_for_shortcut(shortcut);
    if (option.accept_value()) {
      std::string value = i == length - 1 ? "" : name.substr(i + 1);
      add_long_option(option.name(), value);
      break;
    } else {
      add_long_option(option.name(), "");
    }
  }
}

void ArgvInput::parse_long_option(const std::string &token) {
  auto name = token.substr(2);
  auto pos = name.find("=");
  // std::cout << "name: " << name << std::endl;
  if (std::string::npos != pos) {
    auto value = name.substr(pos + 1);
    if (0 == value.size()) {
      parsed_.emplace_back(value);
    }
    add_long_option(name.substr(0, pos), value);
  } else {
    add_long_option(name, "");
  }
}

void ArgvInput::parse_argument(const std::string &token) {

  // Definition argument is empty then not parse.
  if (definition_->get_argument_count() <= 0) {
#if _DEBUG
    std::string str = "ArgvInput::parse_argument \"" + token + "\"" + " empty";
    std::cout << str << std::endl;
#endif
    return;
  }

  auto c = arguments_.size();
  // if input is expecting another argument, add it
  if (definition_->has_argument(c)) {
    auto arg = definition_->get_argument(c);
    arguments_[arg.name()] = (arg.is_array() ? "#" : "") + token; 
  } else if (definition_->has_argument(c - 1) && 
      definition_->get_argument(c - 1).is_array()) {
    auto arg = definition_->get_argument(c - 1);
    arguments_[arg.name()] += "#" + token;
  } else { // unexpected argument
    auto all = definition_->get_arguments();
    std::string command_name{""};
    auto it = all.begin();
    if (it != all.end()) {
      auto input_argument = it->second;
      if ("command" == input_argument.name()) {
        command_name = arguments_["command"];
        all.erase(it);
      }
    }
    std::string e;
    if (all.size() > 0) {
      if (command_name != "") {
        e = "Too many arguments to \"" + command_name 
          + "\" command, expected arguments \"" 
          + implode(" ", array_keys(all)) + "\".";
      } else {
        e = "Too many arguments, expected arguments \"" 
          + implode(" ", array_keys(all)) + "\".";
      }
    } else if (command_name != "") {
      e = "No arguments expected for \"" 
        + command_name + "\" command, got \"" + token + "\".";
    } else {
      e = "No arguments expected, got \"" + token + "\".";
    }
    throw std::runtime_error(e);
  }
}

void ArgvInput::add_short_option(
    const std::string &shortcut, const std::string &value) {
  if (!definition_->has_shortcut(shortcut)) {
    std::string e = "The \"-" + shortcut + "\" option does not exist.";
    throw std::runtime_error(e);
  }
  add_long_option(definition_->get_option_for_shortcut(shortcut).name(), value);
}

void ArgvInput::add_long_option(
    const std::string &name, const std::string &_value) {
  if (!definition_->has_option(name)) {
    std::string e = "The \"--" + name + "\" option does not exist.";
    throw std::runtime_error(e);
  }
  std::string value{_value};
  auto option = definition_->get_option(name);
  if (value != "" && !option.accept_value()) {
    std::string e = "The \"--" + name + "\" option does not accept a value.";
    throw std::runtime_error(e);
  }
  if (value == "" && option.accept_value() && parsed_.size() > 0) {
    // if option accepts an optional or mandatory argument 
    // let's see if there is one provided
    auto next = parsed_.back(); parsed_.pop_back();
    if ((next[0] != ' ' && next[0] != '-') || "" == next) {
      value = next;
    } else {
      parsed_.emplace_back(next);
    }
  }
  if ("" == value) {
    if (option.is_required()) {
      std::string e = "The \"--" + name + "\" option requires a value.";
      throw std::runtime_error(e);
    }
    if (!option.is_array() && !option.is_optional())
      value = "1";
  }
  if (option.is_array()) {
    options_[name] += "#" + value;
  } else {
    options_[name] = value;
  }
}

std::string ArgvInput::get_first_argument() const {
  bool is_option{false};
  auto size = tokens_.size();
  for (decltype(size) i = 0; i < size; ++i) {
    auto token = tokens_[i];
    if (token != "" && "-" == token) {
      if (token.find("=") != std::string::npos || 
          (i + 1 >= size || tokens_[i + 1] == "")) {
        continue;
      }
      // If it's a long option, consider that everything 
      // after "--" is the option name.
      // Otherwise, use the last char (if it's a short option set, 
      // only the last one can take a value with space separator)
      std::string name = 
        '-' == token[1] ? token.substr(2) : token.substr(0, size - 1);
      if ("" == options_[name] && !definition_->has_shortcut(name)) {
        
      } else if ((options_[name] != "" && 
            options_[(name = definition_->shortcut_toname(name))] != "") && 
          tokens_[i + 1] == options_[name]) {
        is_option = true;
      }
      continue;
    }
    if (is_option) {
      is_option = false;
      continue;
    }
    return token;
  }
  return "";
}

bool ArgvInput::has_parameter_option(
    const std::vector<std::string> &values, bool only_params) const {
  for (const auto &token : tokens_) {
    if (only_params && "--" == token) return false;
    for (const auto &value : values) {
      // Options with values: 
      //  For long options, test for '--option=' at beginning
      //  For short options, test for '-o' at beginning
      std::string leading = 0 == value.find("--") ? value + "=" : value;
      if (token == value || (leading != "" && 0 == token.find(leading))) {
        return true;
      }
    }
  }
  return false;
}

type::variable_t 
ArgvInput::get_parameter_option(
    const std::vector<std::string> &values, 
    type::variable_t def, bool only_params) const {
  std::list<std::string> tokens;
  for (const std::string &token : tokens_)
    tokens.emplace_back(token);
  while (0 < tokens.size()) {
    auto token = tokens.front(); tokens.pop_front();
    if (only_params && "--" == token) return def;
    for (const auto &value : values) {
      if (token == value) {
        return tokens.front();
      }
      // Options with values:
      //  For long options, test for '--option=' at beginning
      //  For short options, test for '-o' at beginning
      auto leading = 0 == value.find("--") ? value + "=" : value;
      if (leading != "" && 0 == token.find(leading))
        return token.substr(leading.size());
    }
  }
  return def;
}

std::string ArgvInput::to_string() const {
  std::vector<std::string> tokens;
  std::regex reg("{^(-[^=]+=)(.+)}");
  std::smatch match;
  for (const auto &token : tokens_) {
    if (std::regex_match(token, match, reg)) {
      tokens.emplace_back(match.str(1) + escape_token(match.str(2)));
      continue;
    }
    if (token != "" && token[0] != '-') {
      tokens.emplace_back(escape_token(token));
      continue;
    }
    tokens.emplace_back(token);
  }
  return implode(" ", tokens);
}
