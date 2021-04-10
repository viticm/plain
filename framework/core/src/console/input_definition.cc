#include <algorithm>
#include "pf/support/helpers.h"
#include "pf/console/input_definition.h"

using namespace pf_console;

// Sets the definition of the input. 
void InputDefinition::set_definition(
    const std::vector<InputParameter *> &defines) {
  std::vector<InputOption> options;
  std::vector<InputArgument> arguments;
  for (InputParameter *define : defines) {
    if (instanceof(define, InputOption)) {
      InputOption option = *dynamic_cast<InputOption *>(define);
      options.emplace_back(option);
    } else {
      InputArgument argument = *dynamic_cast<InputArgument *>(define);
      arguments.emplace_back(argument);
    }
  }
  set_options(options);
  set_arguments(arguments);
}

// Sets the InputArgument objects.
void InputDefinition::set_arguments(
    const std::vector<InputArgument> &arguments) {
  arguments_.clear();
  required_count_ = 0;
  has_optional_ = false;
  has_an_array_argument_ = false;
  add_arguments(arguments);
}

// Adds an array of InputArgument objects.
void InputDefinition::add_arguments(
    const std::vector<InputArgument> &arguments) {
  for (InputArgument argument : arguments) {
    add_argument(argument);
  }
}

// Add an argument.
void InputDefinition::add_argument(const InputArgument &argument) {
  if (arguments_.find(argument.name()) != arguments_.end()) {
    std::string msg{""};
    msg = "An argument with name '" + argument.name() + "' already exists.";
    AssertEx(false, msg.c_str());
    return;
  }
  if (has_an_array_argument_) {
    AssertEx(false, "Cannot add an argument after an array argument.");
    return;
  }
  if (argument.is_required() && has_optional_) {
    AssertEx(false, "Cannot add a required argument after an optional one.");
    return;
  }
  if (argument.is_array()) {
    has_an_array_argument_ = true;
  }
  if (argument.is_required()) {
    ++required_count_;
  } else {
    has_optional_ = true;
  }
  argument_names_.emplace_back(argument.name());
  arguments_[argument.name()] = argument;
}

// Sets an argument.
void InputDefinition::set_argument(const InputArgument &argument) {
  if (arguments_.find(argument.name()) == arguments_.end()) {
    AssertEx(false, "Cannot set a not exists argument.");
    return;
  }
  arguments_[argument.name()] = argument;
}

// Get an argument by name.
InputArgument InputDefinition::get_argument(const std::string &name) {
  InputArgument r;
  if (!has_argument(name)) {
    std::string msg{""};
    msg = "The " +name+ " argument does not exist.";
    AssertEx(false, msg.c_str());
    return r;
  }
  r = arguments_[name];
  return r;
}

// Get an argument by position.
InputArgument InputDefinition::get_argument(uint32_t pos) {
  InputArgument r;
  if (pos < argument_names_.size())
    r = arguments_[argument_names_[pos]];
  return r;
}

// Returns true if an InputArgument object exists by name.
bool InputDefinition::has_argument(const std::string &name) {
  return arguments_.find(name) != arguments_.end();
}

// Returns true if an InputArgument object exists by position.
bool InputDefinition::has_argument(uint32_t pos) {
  return pos < argument_names_.size();
}

// Gets the array of InputArgument objects.
std::map<std::string, InputArgument> InputDefinition::get_arguments() {
  return arguments_;
}

// Returns the number of InputArguments.
size_t InputDefinition::get_argument_count() const {
  return arguments_.size();
}

// Returns the number of required InputArguments.
size_t InputDefinition::get_argument_require_count() const {
  return static_cast<size_t>(required_count_);
}

// Gets the default values.
std::map<std::string, std::string> InputDefinition::get_argument_defaults() {
  std::map<std::string, std::string> r;
  for (auto it = arguments_.begin(); it != arguments_.end(); ++it) {
    r[it->first] = it->second.get_default();
  }
  return r;
}

// Sets the InputOption objects.
void InputDefinition::set_options(const std::vector<InputOption> &options) {
  options_.clear();
  shortcuts_.clear();
  add_options(options);
}

// Adds an array of InputOption objects.
void InputDefinition::add_options(const std::vector<InputOption> &options) {
  for (const InputOption &option : options) {
    add_option(option);
  }
}

// Add an option.
void InputDefinition::add_option(const InputOption &option) {
  using namespace pf_support;
  if (options_.find(option.name()) != options_.end() &&
      option.equals(&options_.find(option.name())->second)) {
    std::string msg{""};
    msg = "An option named " + option.name() + " already exists.";
    AssertEx(false, msg.c_str());
    return;
  }
  auto shortcuts = option.shortcut();
  // std::cout << "name: " << option.name() << " shortcuts: " << shortcuts << std::endl;
  if ("" != shortcuts) {
    for (auto &shortcut : explode("|", shortcuts)) {
      if (shortcuts_.find(shortcut.data) != shortcuts_.end() &&
          option.equals(&options_.find(shortcuts_[shortcut.data])->second)) {
        std::string msg{""};
        msg = "An option with shortcut "+shortcut.data+" already exists.";
        AssertEx(false, msg.c_str());
        return;
      }
    }
  }

  options_[option.name()] = option;
  if ("" != shortcuts) {
    for (auto &shortcut : explode("|", shortcuts)) {
      shortcuts_[shortcut.data] = option.name();
    }
  }
}

// Returns an InputOption by name.
InputOption InputDefinition::get_option(const std::string &name) {
  InputOption r;
  if (!has_option(name)) {
    std::string msg{""};
    msg = "The '--" + name + "' option does not exist.";
    AssertEx(false, msg.c_str());
    return r;
  }
  r = options_[name];
  return r;
}

// Returns true if an InputOption object exists by name.
bool InputDefinition::has_option(const std::string &name) const {
  return options_.find(name) != options_.end();
}

// Gets the array of InputOption objects. 
std::map<std::string, InputOption> InputDefinition::get_options() {
  return options_;
}

// Returns true if an InputOption object exists by shortcut.
bool InputDefinition::has_shortcut(const std::string &name) {
  return shortcuts_.find(name) != shortcuts_.end();
}

// Gets an InputOption by shortcut.
InputOption InputDefinition::get_option_for_shortcut(
    const std::string &shortcut) {
  return get_option(shortcut_toname(shortcut));
}

// Gets an array of default values.
std::map<std::string, std::string> InputDefinition::get_option_defaults() {
  std::map<std::string, std::string> r;
  for (auto it = options_.begin(); it != options_.end(); ++it)
    r[it->first] = it->second.get_default();
  return r;
}

// Returns the InputOption name given a shortcut.
std::string InputDefinition::shortcut_toname(const std::string &shortcut) {
  if (shortcuts_.find(shortcut) == shortcuts_.end()) {
    std::string msg{""};
    msg = "The -" +shortcut+ " option does not exist.";
    AssertEx(false, msg.c_str());
    return "";
  }
  return shortcuts_[shortcut];
}

// Gets the synopsis.
std::string InputDefinition::get_synopsis(bool _short) {
  using namespace pf_support;
  std::vector<std::string> elements;
  if (_short && !options_.empty()) {
    elements.emplace_back("[options]");
  } else if (!_short) {
    for (auto it = options_.begin(); it != options_.end(); ++it) {
      std::string value{""};
      if (it->second.accept_value()) {
        std::string temp{it->second.name()};
        std::transform(
          temp.begin(), temp.end(), temp.begin(), (int (*)(int))std::toupper);
          value = value + " " + (it->second.is_optional() ? "[" : "")
                  + temp + (it->second.is_optional() ? "[" : "");
      }
      std::string _shortcut = it->second.shortcut();
      std::string shortcut{""};
      if (_shortcut != "") shortcut = shortcut + "-" + _shortcut + "|";
      std::string element{"["};
      element += shortcut + "--" + it->second.name() + value + "]";
      elements.emplace_back(element);
    }
  }

  if (elements.size() > 0 and arguments_.size() > 0) {
    elements.emplace_back("[--]");
  }

  std::string tail{""};

  for (auto it = arguments_.begin(); it != arguments_.end(); ++it) {
    std::string element = "<" + it->second.name() + ">";
    if (it->second.is_array()) {
      element += "...";
    }
    if (!it->second.is_required()) {
      element = "[" + element;
      tail += "]";
    }
    elements.emplace_back(element);
  }
  return implode(" ", elements) + tail;
}
