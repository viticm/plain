/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id input_option.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2020 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/01/06 15:19
 * @uses The input option of console.
 */
#ifndef PF_CONSOLE_INPUT_OPTION_H_
#define PF_CONSOLE_INPUT_OPTION_H_

#include "pf/console/config.h"
#include "pf/interfaces/console/input_parameter.h"
#include "pf/sys/assert.h"

namespace pf_console {

class InputOption : public pf_interfaces::console::InputParameter {
 
 public:

   InputOption() {}
   
   InputOption(const std::string &_name, 
               const std::string &shortcut = "",
               uint16_t mode = kModeNone, 
               const std::string &description = "",
               const std::string &def = "") : InputParameter(
                 _name, mode, description, def
                 ) {
     std::string name{_name};
     if ("--" == name.substr(0, 1)) {
       name = name.substr(2);
     }
     if ("" == name) {
       AssertEx(false, "An option name cannot be empty.");
     }
     if ("" != shortcut) {
        set_shortcut(shortcut);
     }
   }

   InputOption(const std::string &_name, 
               const std::string &shortcut = "",
               uint16_t mode = kModeNone, 
               const std::string &description = "",
               const std::vector<std::string> &def = {}) : InputParameter(
                 _name, mode, description, def
                 ) {
     std::string name{_name};
     if ("--" == name.substr(0, 1)) {
       name = name.substr(2);
     }
     if ("" == name) {
       AssertEx(false, "An option name cannot be empty.");
     }
     if ("" != shortcut) {
        set_shortcut(shortcut);
     }
   }

   virtual ~InputOption() {}

 public:

   // Returns the option shortcut.
   virtual std::string shortcut() const { return shortcut_; }

 protected:

   void set_shortcut(const std::string &shortcut);

 private:

   std::string shortcut_;

};

} // namespace pf_console

#endif //PF_CONSOLE_INPUT_OPTION_H_
