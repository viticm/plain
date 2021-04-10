/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id input.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/02 15:36
 * @uses Input is the base class for all concrete Input classes.
 *       * `ArgvInput`: The input comes from the CLI arguments (argv)
 *       * `StringInput`: The input is provided as a string
 *       * `ArrayInput`: The input is provided as an array
 */
#ifndef PF_CONSOLE_INPUT_H_
#define PF_CONSOLE_INPUT_H_

#include "pf/console/config.h"
#include "pf/basic/type/variable.h"
#include "pf/console/input_definition.h"

namespace pf_console {

class Input {

 public:
   Input(InputDefinition *definition = nullptr);
   virtual ~Input() {}

 public:

   using variable_t = pf_basic::type::variable_t;

 public:

   // Bind definition.
   void bind(InputDefinition *definition, bool need_parse = true);

   // Aalidate arguments.
   void validate() const;

   // Get interactive.
   bool is_interactive() const {
     return interactive_;
   }

   // Set interactive.
   void set_interactive(bool interactive) {
     interactive_ = interactive;
   }

   // Get arguments.
   std::map<std::string, std::string> get_arguments() const;

   // Get argument.
   std::string get_argument(const std::string &name) const;

   // Set argument.
   void set_argument(const std::string &name, const std::string &value);

   // Has argument.
   bool has_argument(const std::string &name) {
     return definition_->has_argument(name);
   }

   // Get options.
   std::map<std::string, std::string> get_options() const;

   // Get option.
   std::string get_option(const std::string &name) const;

   // Set option.
   void set_option(const std::string &name, const std::string &value);

   // Has option.
   bool has_option(const std::string &name) const {
     return definition_->has_option(name);
   }

   // Escapes a token through escapeshellarg if it contains unsafe chars.
   std::string escape_token(const std::string &token) const;

   // Has parameter option.
   virtual bool has_parameter_option(const std::vector<std::string> &values, 
       bool only_params = false) const {
      return false;
   }
   // Get parameter option.
   virtual variable_t get_parameter_option(
       const std::vector<std::string> &values, 
       variable_t def = false,
       bool only_params = false) const {
     return "";
   }

   // Get first argument.
   virtual std::string get_first_argument() const {
     return "";
   }

 protected:

   // Processes command line arguments. 
   virtual void parse() = 0;

 protected:

   bool interactive_;
   std::unique_ptr<InputDefinition> definition_;
   // If an argument value is array, then it split with '#'.
   std::map<std::string, std::string> arguments_;
   // If an option value is array, then it split wtih '#'.
   mutable std::map<std::string, std::string> options_;

};

} // namespace pf_console

#endif // PF_CONSOLE_INPUT_H_
