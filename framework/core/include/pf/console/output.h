/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id output.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/26 14:39
 * @uses The output class of console.
 */

#ifndef PF_CONSOLE_OUTPUT_H_
#define PF_CONSOLE_OUTPUT_H_

#include "pf/console/config.h"
#include "pf/interfaces/console/output.h"

namespace pf_console {

class Output : public pf_interfaces::console::Output {

 public:

   Output() {}
   virtual ~Output() {}

 public:
   
   // Writes a message to the output.
   virtual void write(const std::string &messages, 
              bool newline = false, uint16_t options = 0);

   // Writes a message to the output and adds a newline at the end.
   virtual void write_ln(const std::string &messages, uint16_t options = 0) {
     write(messages, true, options);
   }


};

} // namespace pf_console

#endif // PF_CONSOLE_OUTPUT_H_
