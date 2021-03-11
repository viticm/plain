#include "pf/console/command.h"

using namespace pf_console;

uint8_t Command::run(Input *input, Output *output) {
  uint8_t r{0};
  // bind the input against the command specific arguments/options
  try {
    input->bind(definition_.get());
  } catch (std::exception &e) {
    if (!ignore_validation_errors_)
      throw e;
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
