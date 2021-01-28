/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id input_argument.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2020 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2020/12/25 11:03
 * @uses The input argument of console.
 */
#ifndef PF_CONSOLE_INPUT_ARGUMENT_H_
#define PF_CONSOLE_INPUT_ARGUMENT_H_

#include "pf/console/config.h"
#include "pf/interfaces/console/input_parameter.h"

namespace pf_console {

class InputArgument : public pf_interfaces::console::InputParameter {

 public:
   
   InputArgument(const std::string &name, 
                 mode_t mode = kModeNone, 
                 const std::string &description = "",
                 const std::string &def = "") {
   }

   InputArgument(const std::string &name, 
                 mode_t mode = kModeNone, 
                 const std::string &description = "",
                 const std::vector<std::string> &def = {}) {}

   virtual ~InputArgument() {}

   virtual const std::string class_name() const {
     return "InputArgument";
   }


};

} // namespace pf_console

#endif //PF_CONSOLE_INPUT_ARGUMENT_H_
