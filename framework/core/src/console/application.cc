#include "pf/basic/string.h"
#include "pf/console/argv_input.h"
#include "pf/console/array_input.h"
#include "pf/console/commands/app.h"
#include "pf/console/commands/help.h"
#include "pf/console/commands/list.h"
#include "pf/basic/logger.h"
#include "pf/console/application.h"

using namespace pf_console;
using namespace pf_interfaces::console;
using namespace pf_basic::string;

uint8_t Application::run(Input *input, Output *output) {
  std::unique_ptr<Input> input_temp;
  std::unique_ptr<Output> output_temp;
  if (is_null(input)) {
    unique_move(Input, new ArgvInput(), input_temp);
    input = input_temp.get();
  }
  if (is_null(output)) {
    unique_move(Output, new Output(), output_temp);
    output = output_temp.get();
  }
  configure_IO(input, output);
  uint8_t exit_code{0};
  try {
    exit_code = do_run(input, output);
  } catch (std::exception &e) {
    std::cout << "Application::run get error!!!: " << e.what() << std::endl;
  }
  return exit_code;
}

uint8_t Application::do_run(Input *input, Output *output) {
  if (input->has_parameter_option({"--version", "-V"})) {
    output->write_ln(get_long_version());
    return 0;
  }
  try {
    input->bind(get_definition());
  } catch(...) {

  }
  auto name = get_command_name(input);
  std::unique_ptr<Input> input_temp;
  if (input->has_parameter_option({"--help", "-h"}, true)) {
    if (name == "") {
      name = "help";
      unique_move(Input,
          new ArrayInput({{"command_name", default_command_name_}}),
          input_temp);
      input = input_temp.get();
    } else {
      want_helps_ = false;
    }
  }
  if (name == "") {
    name = default_command_name_;
    auto definition = get_definition();
    definition->set_argument(InputArgument("command",
        InputArgument::kModeOptional,
        definition->get_argument("command").get_description(), name));
  }
  Command *command{nullptr};
  try {
    running_command_ = nullptr;
    command = find(name);
  } catch (...) {

  }
  if (is_null(command)) {
    FAST_ERRORLOG(CONSOLE_MODULENAME, 
                  "[console] (Application::run)"
                  " can't find the command: %s", 
                  name.c_str());
    return 1;
  }
  running_command_ = command;
  auto exit_code = do_runcommand(command, input, output);
  return exit_code;
}

InputDefinition *Application::get_definition() {
  if (is_null(definition_)) {
    std::unique_ptr<InputDefinition> temp(new InputDefinition());
    *temp = get_default_input_definition();
    definition_ = std::move(temp);
    if (single_command_) {
      if (is_null(definition_temp_))    {
        unique_move(InputDefinition, new InputDefinition(), definition_temp_);
        *definition_temp_ = *definition_;
      }
      definition_temp_->set_arguments({});
      return definition_temp_.get();
    }
  }
  return definition_.get();
}

std::string Application::get_long_version() const {
  std::string r{"Console Tool"};
  if (name_ != "") {
    if (version_ != "") {
      r = name_ + " " + version_;
    }
    r = name_;
  }
  return r;
}

Command *Application::add(Command *command) {
  // std::cout << "Add command: " << command->name() << std::endl;
  if (!command->is_enabled()) {
    return nullptr;
  }
  init();
  command->set_application(this);
  // Will throw if the command is not correctly initialized.
  command->get_definition();
  command->configure();
  auto name = command->name();
  if (name == "") {
    throw std::logic_error("command cannot have an empty name.");
  }
  if (commands_.find(name) != commands_.end()) {
    return commands_[name].get();
  }
  std::unique_ptr<Command> temp;
  unique_move(Command, command, temp);
  commands_[name] = std::move(temp);
  for (auto const &alias : command->get_aliases()) {
    command_aliases_[alias] = name;
  }
  return command;
}

Command *Application::get(const std::string &_name) {
  auto name = get_command_real_name(_name);
  if ("" == name) {
    std::string e = "The command \"" + _name + "\" does not exist.";
    throw std::invalid_argument(e);
  }
  if (commands_.find(name) == commands_.end()) return nullptr;
  auto command = commands_[name].get();
  if (want_helps_) {
    want_helps_ = false;
    auto help_command = get("help");
    help_command->set_command(command);
    return help_command;
  }
  return command;
}

std::vector<std::string> Application::get_namespaces() {
  std::vector<std::string> r;
  auto commands = all();
  for (auto it = commands.begin(); it != commands.end(); ++it) {
    if (it->second->is_hidden()) continue;
    auto temp = extract_all_namespace(it->first);
    for (const auto &one : temp) r.emplace_back(one);
    for (const auto &alias : it->second->get_aliases()) {
      auto temp1 = extract_all_namespace(alias);
      for (const auto &one : temp1) r.emplace_back(one);
    }
  }
  // * The result maybe need use array_unique to remove the same values.
  return r;
}

std::string Application::find_namespace(const std::string &_namespace) {
  return "";
}

Command *Application::find(const std::string &name) {
  init();
  for (auto it = commands_.begin(); it != commands_.end(); ++it) {
    if (!is_null(it->second)) {
      for (const auto &alias : it->second->get_aliases()) {
        if ("" == command_aliases_[alias]) 
          command_aliases_[alias] = it->second->name();
      }
    } else {
      std::cout << "find no command: " << it->first << std::endl;
    }
  }
  return get(name);
}

std::map<std::string, Command *>
Application::all(const std::string &_namespace) {
  std::map<std::string, Command *> r;
  init();
  if ("" == _namespace) {
    for (auto it = commands_.begin(); it != commands_.end(); ++it) {
      r[it->first] = it->second.get();
    }
    return r;
  }
  for (auto it = commands_.begin(); it != commands_.end(); ++it) {
    if (_namespace == extract_namespace(it->first)) {
      r[it->first] = it->second.get();
    }
  }
  return r;
}

Application &Application::set_default_command(
    const std::string &name, bool is_single_command) {
  default_command_name_ = name;
  if (is_single_command) {
    // Ensure the command exist
    find(name);
    single_command_ = true;
  }
  return *this;
}

uint8_t Application::do_runcommand(
    Command *command, Input *input, Output *output) {
  // std::cout << "do_runcommand: " << command->name() << std::endl;
  return command->run(input, output);
}

std::string Application::get_command_name(Input *input) const {
  return single_command_ ? default_command_name_ : input->get_first_argument();
}

InputDefinition Application::get_default_input_definition() const {
  std::vector<InputParameter *> p;
  std::unique_ptr<InputParameter> p1(new InputArgument(
        "command", InputParameter::kModeRequired,
        "The command to execute", ""
        ));
  p.emplace_back(p1.get());
  std::unique_ptr<InputParameter> p2(new InputOption(
        "--help", "-h", InputParameter::kModeNone,
        "Display help for the given command. When no command"
        " is given display help for the" + default_command_name_ + "command", ""
        ));
  p.emplace_back(p2.get());
  std::unique_ptr<InputParameter> p3(new InputOption(
        "--quiet", "-q", InputParameter::kModeNone,
        "Do not output any message", ""
        ));
  p.emplace_back(p3.get());
  std::unique_ptr<InputParameter> p4(new InputOption(
        "--verbose", "-v|vv|vvv", InputParameter::kModeNone,
        "Increase the verbosity of messages: 1 for normal output, "
        "2 for more verbose output and 3 for debug", ""
        ));
  p.emplace_back(p4.get());
  std::unique_ptr<InputParameter> p5(new InputOption(
        "--version", "-V", InputParameter::kModeNone,
        "Display this application version", ""
        ));
  p.emplace_back(p5.get());
  std::unique_ptr<InputParameter> p6(new InputOption(
        "--ansi", "", InputParameter::kModeNone,
        "Force ANSI output", ""
        ));
  p.emplace_back(p6.get());
  std::unique_ptr<InputParameter> p7(new InputOption(
        "--no-ansi", "", InputParameter::kModeNone,
        "Disable ANSI output", ""
        ));
  p.emplace_back(p7.get());
  std::unique_ptr<InputParameter> p8(new InputOption(
        "--no-interaction", "-n", InputParameter::kModeNone,
        "Do not ask any interactive question", ""
        ));
  p.emplace_back(p8.get());

  return InputDefinition(p);
}

std::vector<Command *> Application::get_default_commands() const {
  std::vector<Command *> r;
  return r;
}

std::string Application::get_abbreviation_suggestions(
    const std::vector<std::string> &abbrevs) const {
  return "";
}

std::vector<std::string> Application::find_alternatives(
    const std::string &name, const std::vector<std::string> &collection) const {
  return {};
}

void Application::configure_IO(Input *input, Output *output) {
  if (input->has_parameter_option({"--ansi"}, true)) {
    output->set_decorated(true);
  } else if (input->has_parameter_option({"'--no-ansi'"}, true)) {
    output->set_decorated(false);
  }
}

std::vector<std::string> Application::extract_all_namespace(
    const std::string &name) {
  std::vector<std::string> r;
  std::vector<std::string> parts;
  explode(name.c_str(), parts, ":", true, true);
  for (const auto &part : parts) {
    if (r.size() > 0) {
      std::string temp = r[r.size() - 1] + ":" + part;
      r.emplace_back(temp);
    } else {
      r.emplace_back(part);
    }
  }
  return r;
}

std::string Application::extract_namespace(
    const std::string &name, int32_t limit) const {
  return "";
}

void Application::init() {
  if (initialized_) return;
  initialized_ = true;
  // std::cout << "Application::init" << std::endl;
  add(new commands::Help());
  add(new commands::List());
  add(new commands::App());
}
