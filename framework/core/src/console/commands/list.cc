#include "pf/console/input_definition.h"
#include "pf/console/application.h"
#include "pf/console/commands/list.h"

using namespace pf_console;
using namespace pf_interfaces::console;
using namespace pf_console::commands;

void List::configure() {
  std::vector< std::shared_ptr<InputParameter> > p;
  std::unique_ptr<InputParameter> p1(new InputArgument(
        "namespace", InputParameter::kModeOptional, 
        "The namespace name", "list"
        ));
  p.emplace_back(std::move(p1));
  std::unique_ptr<InputParameter> p2(new InputOption(
        "raw", "", InputParameter::kModeNone, "To output raw command help", ""
        ));
  p.emplace_back(std::move(p2));
  
  std::unique_ptr<InputDefinition> definition(new InputDefinition(p));
  set_name("list")
    .set_definition(definition)
    .set_description("Lists commands")
    .set_help("List the all can use command.");
}

uint8_t List::execute(Input *input, Output *output) {
  UNUSED(input);
  output->write_ln("The command list:");
  auto app = get_application();
  if (!is_null(app)) {
    auto all = app->all();
    for (auto it = all.begin(); it != all.end(); ++it) {
      output->write_ln(it->first + ": " + it->second->get_description());
    }
  }
  return kCommandExecuteNone;
}
