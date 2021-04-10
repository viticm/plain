/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id input_definition.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2020 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2020/12/25 19:51
 * @uses The input definition class.
 */
#ifndef PF_CONSOLE_INPUT_DEFINITION_H_
#define PF_CONSOLE_INPUT_DEFINITION_H_

#include "pf/console/config.h"
#include "pf/interfaces/console/input_parameter.h"
#include "pf/console/input_argument.h"
#include "pf/console/input_option.h"

namespace pf_console {

class InputDefinition {

 public:

   using InputParameter = pf_interfaces::console::InputParameter;

 public:
   InputDefinition(const std::vector<InputParameter *> &defines = {}) {
     required_count_ = 0;
     has_optional_ = false;
     has_an_array_argument_ = false;
     set_definition(defines);
   }
   virtual ~InputDefinition() {
     // std::cout << "InputDefinition destruct" << std::endl;
   }

 public:

   // Sets the definition of the input.
   void set_definition(const std::vector<InputParameter *> &defines);

   // Sets the InputArgument objects.
   void set_arguments(const std::vector<InputArgument> &arguments);

   // Adds an array of InputArgument objects.
   void add_arguments(const std::vector<InputArgument> &arguments);

   // Adds an argument.
   void add_argument(const InputArgument &argument);

   // Sets an argument.
   void set_argument(const InputArgument &argument);

   // Get an argument by name.
   InputArgument get_argument(const std::string &name);

   // Get an argument by position.
   InputArgument get_argument(uint32_t pos);

   // Returns true if an InputArgument object exists by name.
   bool has_argument(const std::string &name);

   // Returns true if an InputArgument object exists by position.
   bool has_argument(uint32_t pos);

   // Gets the array of InputArgument objects.
   std::map<std::string, InputArgument> get_arguments();

   // Returns the number of InputArguments.
   size_t get_argument_count() const;

   // Returns the number of required InputArguments.
   size_t get_argument_require_count() const;

   // Gets the default values.
   std::map<std::string, std::string> get_argument_defaults();

   // Sets the InputOption objects.
   void set_options(const std::vector<InputOption> &options);

   // Adds an array of InputOption objects.
   void add_options(const std::vector<InputOption> &options);

   // Add a option.
   void add_option(const InputOption &option);

   // Returns an InputOption by name.
   InputOption get_option(const std::string &name);

   // Returns true if an InputOption object exists by name.
   bool has_option(const std::string &name) const;

   // Gets the array of InputOption objects.
   std::map<std::string, InputOption> get_options();

   // Returns true if an InputOption object exists by shortcut.
   bool has_shortcut(const std::string &name);

   // Gets an InputOption by shortcut.
   InputOption get_option_for_shortcut(const std::string &shortcut);

   // Gets an array of default values.
   std::map<std::string, std::string> get_option_defaults();

   // Returns the InputOption name given a shortcut.
   std::string shortcut_toname(const std::string &shortcut);

   // Gets the synopsis.
   std::string get_synopsis(bool _short = false);

 private:

   std::map<std::string, InputArgument> arguments_;
   std::vector<std::string> argument_names_;
   uint32_t required_count_;
   bool has_an_array_argument_;
   bool has_optional_;
   std::map<std::string, InputOption> options_;
   std::map<std::string, std::string> shortcuts_;

};

} // namespace pf_console

#endif //PF_CONSOLE_INPUT_DEFINITION_H_
