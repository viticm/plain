/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id application.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/04/08 14:17
 * @uses The application command.
 */

#ifndef PF_CONSOLE_COMMANDS_APP_H_
#define PF_CONSOLE_COMMANDS_APP_H_

#include "pf/console/commands/config.h"
#include "pf/console/command.h"

namespace pf_console {

namespace commands {

class App : public Command {

 public:
   App(const std::string &name = "app") : Command(name) {}
   virtual ~App() {}

 public:

   virtual void configure();
 protected:

   virtual uint8_t execute(Input *input, Output *output);

};

} // namespace commands

} // namespace pf_console

#endif // PF_CONSOLE_COMMANDS_APP_H_
