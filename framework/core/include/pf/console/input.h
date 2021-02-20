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
#include "pf/console/input_definition.h"

namespace pf_console {

class Input {

 public:
   Input(InputDefinition *definition = nullptr) {}
   virtual ~Input() {}

 public:

   // Bind definition.
   void bind(const InputDefinition &definition);

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
   std::map<std::string, InputArgument> get_arguments();

   // Get argument.
   InputArgument get_argument(const std::string &name);

   // Set argument.
   void set_argument(const InputArgument &argument);

   // Has argument.
   bool has_argument(const std::string &name);

   // Get options.
   std::map<std::string, InputOption> get_options();

   // Get option.
   std::string get_option() const;

   // Set option.
   void set_option(const std::string &name, const std::string &value);

   // Has option.
   bool has_option() const;

   // Escapes a token through escapeshellarg if it contains unsafe chars.
   std::string escape_token(const std::string &token);


 protected:

   // Processes command line arguments. 
   virtual void parse() = 0;

 protected:

   bool interactive_;

};

} // namespace pf_console

#endif // PF_CONSOLE_INPUT_H_
