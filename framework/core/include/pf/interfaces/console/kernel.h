/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id kernel.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2020 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2020/12/18 14:26
 * @uses The plain framework console kernel interface.
 */
#ifndef PF_INTERFACES_CONSOLE_KERNEL_H_
#define PF_INTERFACES_CONSOLE_KERNEL_H_

#include "pf/interfaces/console/config.h"
#include "pf/basic/type/variable.h"

namespace pf_interfaces {

namespace console {

class Kernel {

 public:
   Kernel() {}
   virtual ~Kernel() {}

 public:

   // Run a console command by name.
   int32_t call(const std::string &command, 
                const pf_basic::variable_array_t &params = {}) = 0;

   // Get the output from the last command.
   std::string output() = 0;

   // Get all of the commands registered with the console.
   std::vector<std::string> all() const = 0;

}

} // namespace console

} // namespace pf_interfaces

#endif //PF_INTERFACES_CONSOLE_KERNEL_H_
