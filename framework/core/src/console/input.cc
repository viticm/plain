#include <regex>
#include "pf/support/helpers.h"
#include "pf/basic/string.h"
#include "pf/console/input.h"

using namespace pf_basic::string;
using namespace pf_support;
using namespace pf_console;

Input::Input(InputDefinition *definition) {
  if (is_null(definition)) {
    auto temp = new InputDefinition();
    std::unique_ptr<InputDefinition> pointer(temp);
    definition_ = std::move(pointer);
  } else {
    bind(definition);
    validate();
  }
}

void Input::bind(InputDefinition *definition) {
  arguments_.clear();
  options_.clear();
  auto temp = new InputDefinition();
  *temp = *definition;
  std::unique_ptr<InputDefinition> pointer(temp);
  definition_ = std::move(pointer);
  parse();
}

void Input::validate() const {
  auto missing_arguments = array_filter<std::string>(
    array_keys(definition_->get_arguments()), 
    [this](const std::string &argument){
      return arguments_.find(argument) != arguments_.end() 
        && definition_->get_argument(argument).is_required();
  });
  if (missing_arguments.size() > 0) {
    std::string e = "Not enough arguments (missing: \"" 
      + implode(", ", missing_arguments) + "\").";
    throw std::runtime_error(e);
  }
}

std::map<std::string, std::string> Input::get_arguments() const {
  return array_merge(definition_->get_argument_defaults(), arguments_);
}

std::string Input::get_argument(const std::string &name) const {
  if (!definition_->has_argument(name)) {
    std::string e = "The \"" + name + "\" argument does not exist.";
    throw std::invalid_argument(e);
  }
  auto it = arguments_.find(name);
  return it != arguments_.end() ? 
    it->second : definition_->get_argument(name).get_default();
}

void Input::set_argument(const std::string &name, const std::string &value) {
  if (!definition_->has_argument(name)) {
    std::string e = "The \"" + name + "\" argument does not exist.";
    throw std::invalid_argument(e);
  }
  arguments_[name] = value;
}

std::map<std::string, std::string> Input::get_options() const {
  return array_merge(definition_->get_option_defaults(), options_);
}

std::string Input::get_option(const std::string &name) const {
  if (!definition_->has_option(name)) {
    std::string e = "The \"" + name + "\" option does not exist.";
    throw std::invalid_argument(e);
  }
  auto it = options_.find(name);
  return it != options_.end() ? 
    it->second : definition_->get_option(name).get_default();
}

void Input::set_option(const std::string &name, const std::string &value) {
  if (!definition_->has_option(name)) {
    std::string e = "The \"" + name + "\" option does not exist.";
    throw std::invalid_argument(e);
  }
  options_[name] = value;
}

std::string Input::escape_token(const std::string &token) const {
  std::regex reg("{^[\\w-]+$}");
  return std::regex_match(token, reg) ? 
    token : "'" + str_replace("'", "\\'", token) + "'";
}
