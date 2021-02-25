#include <stdexcept>
#include "pf/support/helpers.h"
#include "pf/console/array_input.h"

using namespace pf_support;
using namespace pf_console;
using namespace pf_basic;

ArrayInput::ArrayInput(
    const std::map<std::string, std::string> &parameters, 
    InputDefinition *definition) : Input(definition) {
  parameters_ = parameters;
}

std::string ArrayInput::get_first_argument() const {
  std::string value{""};
  for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
    if ('-' == it->first[0]) continue;
    value = it->second;
    break;
  }
  return value;
}

bool ArrayInput::has_parameter_option(
    const std::vector<std::string> &values, bool only_params) const {
  for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
    if (only_params && "--" == it->second) return false;
    if (in_array(it->second, values)) return true;
  }
  return false;
}

type::variable_t ArrayInput::get_parameter_option(
    const std::vector<std::string> &values,
    variable_t def,
    bool only_params) const {
  for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
    if (only_params && ("--" == it->first || "--" == it->second))
      return def;
    if (in_array(it->first, values)) return it->second;
  }
  return def;
}

void ArrayInput::parse() {
  for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
    if ("--" == it->first) return;
    if (0 == it->first.find("--")) {
      add_long_option(it->first.substr(2), it->second);
    } else if ('-' == it->first[0]) {
      add_short_option(it->first.substr(1), it->second);
    } else {
      add_argument(it->first, it->second);
    }
  }
}

void ArrayInput::add_argument(
    const std::string &name, const std::string &value) {
  if (!definition_->has_argument(name)) {
    std::string e = "The \"" + name + "\" argument does not exist.";
    throw std::invalid_argument(e);
  }
}

void ArrayInput::add_short_option(
    const std::string &shortcut, const std::string &value) {
  if (!definition_->has_shortcut(shortcut)) {
    std::string e = "The \"-" + shortcut + "\" option does not exist.";
    throw std::invalid_argument(e);
  }
  add_long_option(definition_->get_option_for_shortcut(shortcut).name(), value);
}

void ArrayInput::add_long_option(
    const std::string &name, const std::string &_value) {
  if (!definition_->has_option(name)) {
    std::string e = "The \"--" + name + "\" option does not exist.";
    throw std::invalid_argument(e);
  }
  std::string value{_value};
  auto option = definition_->get_option(name);
  if (value == "") {
    if (option.is_required()) {
      std::string e = "The \"--" + name + "\" option requires a value.";
      throw std::invalid_argument(e);
    }
    if (!option.is_optional()) {
      value = "1";
    }
  }
  options_[name] = value;
}

std::string ArrayInput::to_string() const {
  std::vector<std::string> params;
  for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
    if ('-' == it->first[0]) {
      if (it->second != "") {
        params.emplace_back(it->first + "=" + escape_token(it->second));
      } else {
        params.emplace_back(it->first + "=");
      }
    }
  }
  return implode(" ", params);
}
