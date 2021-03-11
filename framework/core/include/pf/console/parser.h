/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id parser.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/03/01 16:37
 * @uses The parse command of console.
 */
#ifndef PF_CONSOLE_PARSER_H_
#define PF_CONSOLE_PARSER_H_

#include "pf/console/config.h"
#include "pf/console/input_argument.h"
#include "pf/console/input_option.h"

namespace pf_console {

class Parser {

 public:
   Parser() {}
   virtual ~Parser() {}

 public:

   // Parse the given console command definition into an array.
   static std::tuple< 
     std::string, std::vector<InputArgument>, std::vector<InputOption> > 
     parse(const std::string &expression);

 protected:

   // Extract the name of the command from the expression.
   static std::string name(const std::string &expression);

   // Extract all of the parameters from the tokens. 
   static std::tuple< std::vector<InputArgument>, std::vector<InputOption> >
     parameters(const std::vector<std::string> &tokens);

   // Parse an argument expression.
   static InputArgument parse_argument(const std::string &token);

   // Parse an option expression.
   static InputOption parse_option(const std::string &token);

   // Parse the token into its token and description segments.
   static std::vector<std::string> 
     extract_description(const std::string &token);
   

};

} // namespace pf_console

#endif // PF_CONSOLE_PARSER_H_
