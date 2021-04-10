/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id list.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/03/27 13:30
 * @uses The list command.
 */

#ifndef PF_CONSOLE_COMMANDS_LIST_H_
#define PF_CONSOLE_COMMANDS_LIST_H_

#include "pf/console/commands/config.h"
#include "pf/console/command.h"

namespace pf_console {

namespace commands {

class List : public Command {

 public:
   List(const std::string &name = "list") : Command(name) {}
   virtual ~List() {}

 public:

   virtual void configure();
   virtual bool is_parse_input() const {
     return false;
   }

 protected:

   virtual uint8_t execute(Input *input, Output *output);

};

} // namespace commands

} // namespace pf_console

#endif // PF_CONSOLE_COMMANDS_LIST_H_
