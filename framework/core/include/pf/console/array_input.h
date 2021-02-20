/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id array_input.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/19 09:33
 * @uses The array input class.
 */

#ifndef PF_CONSOLE_ARRAY_INPUT_H_
#define PF_CONSOLE_ARRAY_INPUT_H_

#include "pf/console/config.h"
#include "pf/console/input.h"

namespace pf_console {

class ArrayInput : public Input {

 public:
   ArrayInput(const std::vector<std::string> &parameters = {},
             InputDefinition *definition = nullptr);
   ~ArrayInput() {}

 public:

   // Get first argument.
   std::string get_first_argument() const;

   // Has parameter option.
   bool has_parameter_option(const std::vector<std::string> &values, 
                             bool only_params = false) const;
   // Get parameter option.
   bool get_parameter_option(const std::vector<std::string> &values, 
                             bool def = false,
                             bool only_params = false) const;
 protected:

   //Parse.
   void parse();

 private:

   // Add an argument.
   // @throws std::string When argument given doesn't exist
   void add_argument(const std::string &name, const std::string &value);

   // Adds a short option value.
   // @throws std::string When option given doesn't exist
   void add_short_option(const std::string &shortcut, const std::string &value);

   // Adds a long option value.
   // @throws std::string When option given doesn't exist
   // @throws std::string When a required value is missing
   void add_long_option(const std::string &name, const std::string &value);



};

} // namespace pf_console

#endif // PF_CONSOLE_ARRAY_INPUT_H_
