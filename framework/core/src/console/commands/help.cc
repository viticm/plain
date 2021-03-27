#include "pf/console/input_definition.h"
#include "pf/console/application.h"
#include "pf/console/commands/help.h"

using namespace pf_console;
using namespace pf_console::commands;
using namespace pf_interfaces::console;

void Help::configure() {
  ignore_validation_errors();
  std::vector<InputParameter *> p;
  std::unique_ptr<InputParameter> p1(new InputArgument(
        "command_name", InputParameter::kModeOptional, 
        "The command name", "help"
        ));
  p.emplace_back(p1.get());
  std::unique_ptr<InputParameter> p2(new InputOption(
        "raw", "", InputParameter::kModeNone, "To output raw command help", ""
        ));
  p.emplace_back(p2.get());
  
  auto definition = new InputDefinition(p);
  set_name("help")
    .set_definition(definition)
    .set_description("Displays help for a command")
    .set_help("command displays help for a given command:");
}

uint8_t Help::execute(Input *input, Output *output) {
  if (is_null(command_))
    command_ = get_application()->find(input->get_argument("command_name"));
  if (is_null(command_)) return 1;

  // Now just print it.
  std::cout << command_->help() << std::endl;
  
  command_ = nullptr;
  return 0;
}
