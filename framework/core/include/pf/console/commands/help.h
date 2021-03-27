/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id help.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/03/25 15:40
 * @uses The help command of console.
 */

#ifndef PF_CONSOLE_COMMANDS_HELP_H_
#define PF_CONSOLE_COMMANDS_HELP_H_

#include "pf/console/config.h"
#include "pf/console/command.h"

namespace pf_console {

namespace commands {

class Help : public Command {

 public:
   Help(const std::string &name = "") : command_{nullptr} {}
   virtual ~Help() {}

 public:

   virtual void set_command(Command *command) {
     command_ = command;
   }

 protected:

   virtual void configure();

   virtual uint8_t execute(Input *input, Output *output);

 private:

   Command *command_;

};

} // namespace commands

} // namespace pf_console

#endif // PF_CONSOLE_COMMANDS_HELP_H_
