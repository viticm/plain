/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id array_input.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/19 09:33
 * @uses ArrayInput represents an input provided as an array.
 *  Usage:
 *    auto input = new ArrayInput({
 *      'command' = 'foo:bar', 'foo' = 'bar', '--bar' = 'foobar'});
 */

#ifndef PF_CONSOLE_ARRAY_INPUT_H_
#define PF_CONSOLE_ARRAY_INPUT_H_

#include "pf/console/config.h"
#include "pf/basic/type/variable.h"
#include "pf/console/input.h"

namespace pf_console {

class ArrayInput : public Input {

 public:
   ArrayInput(const std::map<std::string, std::string> &parameters = {},
             InputDefinition *definition = nullptr);
   virtual ~ArrayInput() {}

 public:

   using variable_t = pf_basic::type::variable_t;

 public:

   // Has parameter option.
   virtual bool has_parameter_option(const std::vector<std::string> &values, 
                                     bool only_params = false) const;
   // Get parameter option.
   virtual variable_t 
     get_parameter_option(const std::vector<std::string> &values, 
                          variable_t def = false,
                          bool only_params = false) const;

   // Parameters to string.
   std::string to_string() const;

 public:

   // Get first argument.
   virtual std::string get_first_argument() const;

 protected:

   //Parse.
   virtual void parse();

 private:

   // Add an argument.
   // @throws std::invalid_argument When argument given doesn't exist
   void add_argument(const std::string &name, const std::string &value);

   // Adds a short option value.
   // @throws std::invalid_argument When option given doesn't exist
   void add_short_option(const std::string &shortcut, const std::string &value);

   // Adds a long option value.
   // @throws std::invalid_argument When option given doesn't exist
   // @throws std::invalid_argument When a required value is missing
   void add_long_option(const std::string &name, const std::string &value);

 private:

   std::map<std::string, std::string> parameters_;

};

} // namespace pf_console

#endif // PF_CONSOLE_ARRAY_INPUT_H_
