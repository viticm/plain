/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id argv_input.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/18 10:21
 * @uses The argv input class.
 */
#ifndef PF_CONSOLE_ARGV_INPUT_H_
#define PF_CONSOLE_ARGV_INPUT_H_

#include "pf/console/config.h"
#include "pf/basic/type/variable.h"
#include "pf/console/input.h"

namespace pf_console {

class ArgvInput : public Input {

 public:
   ArgvInput(const std::vector<std::string> &argv = {},
             InputDefinition *definition = nullptr);
   virtual ~ArgvInput() {}

 public:

   using variable_t = pf_basic::type::variable_t;

 public:

   // Get the first argument.
   std::string get_first_argument() const;

   // Has parameter option.
   bool has_parameter_option(const std::vector<std::string> &values, 
                             bool only_params = false) const;
   // Get parameter option.
   variable_t get_parameter_option(const std::vector<std::string> &values, 
                                   variable_t def = false,
                                   bool only_params = false) const;

   // Returns a stringified representation of the args passed to the command.
   std::string tostring() const;

 protected:

   // Set the tokens.
   void set_tokens(const std::vector<std::string> &tokens) {
     tokens_ = tokens;
   };

   // Parse all commands.
   void parse();

 private:

   // Parses a short option.
   void parse_short_option(const std::string &token);

   // Parses a short option set.
   void parse_short_option_set(const std::string &name);

   // Parses a long option.
   void parse_long_option(const std::string &token);

   // Parses an argument.
   // @throws std::runtime_error When too many arguments are given
   void parse_argument(const std::string &token);

   // Adds a short option value.
   // @throw std::runtime_error When option given doesn't exist
   void add_short_option(const std::string &shortcut, const std::string &value);

   // Adds a long option value.
   // @throws std::runtime_error When option given doesn't exist
   void add_long_option(const std::string &name, const std::string &value);

 private:

   std::vector<std::string> tokens_;
   std::list<std::string> parsed_;

};

} // namespace pf_console

#endif // PF_CONSOLE_ARGV_INPUT_H_
