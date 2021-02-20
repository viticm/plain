/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id string_input.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/19 10:33
 * @uses StringInput represents an input provided as a string. 
 *       auto input = new StringInput('foo --bar="foobar"');
 */

#ifndef PF_CONSOLE_STRING_INPUT_H_
#define PF_CONSOLE_STRING_INPUT_H_

#include "pf/console/config.h"
#include "pf/console/argv_input.h"

namespace pf_console {

class StringInput : public ArgvInput {

 public:

   StringInput(const std::string &input) : ArgvInput() {
     set_tokens(tokenize(input));
   }
   ~StringInput() {}

 private:

   // Tokenizes a string.
   std::vector<std::string> tokenize(const std::string &input) const;


};

} // namespace pf_console

#endif // PF_CONSOLE_STRING_INPUT_H_
