#include "pf/support/helpers.h"
#include "pf/console/application.h"
#include "pf/console/command.h"

using namespace pf_support;
using namespace pf_console;

// Static member must be initialized.
std::string Command::default_name_{"unknown"};
   
void Command::merge_application_definition(bool merge_args) {
  if (is_null(app_) || !is_null(full_definition_)) return;
  unique_move(InputDefinition, new InputDefinition(), full_definition_);
  full_definition_->set_options(array_values(definition_->get_options()));
  full_definition_->add_options(
    array_values(app_->get_definition()->get_options()));
  if (merge_args) {
    full_definition_->set_arguments(
        array_values(app_->get_definition()->get_arguments()));
    full_definition_->add_arguments(array_values(definition_->get_arguments()));
  } else {
    full_definition_->set_arguments(array_values(definition_->get_arguments()));
  }
}

uint8_t Command::run(Input *input, Output *output) {
  uint8_t r{0};
  merge_application_definition();
  // bind the input against the command specific arguments/options
  try {
    input->bind(get_definition(), is_parse_input());
  } catch (std::exception &e) {
    if (!ignore_validation_errors_)
      throw std::runtime_error(e.what());
  }
  initialize(input, output);

  // Set process title.
  if (!process_title_.empty()) {

  }
  if (input->is_interactive()) {
    interact(input, output);
  }

  // The command name argument is often omitted when a command is executed 
  // directly with its run() method.
  // It would fail the validation if we didn't make sure the command argument 
  // is present, since it's required by the application.
  if (input->has_argument("command") && input->get_argument("command") == "") {
    input->set_argument("command", name_);
  }

  input->validate();

  if (code_) {
    r = code_(input, output);
  } else {
    r = execute(input, output);
  }

  return r;
}
