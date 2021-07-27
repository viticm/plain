/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id output.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/02/27 10:14
 * @uses your description
 */

#ifndef PF_INTERFACES_CONSOLE_OUTPUT_H_
#define PF_INTERFACES_CONSOLE_OUTPUT_H_

#include "pf/interfaces/config.h"

namespace pf_interfaces {

namespace console {

class PF_API Output {

 public:
   Output() {}
   virtual ~Output() {}

 public:

   typedef enum {
     kVerbosityQuiet = 16,
     kVerbosityNormal = 32,
     kVerbosityVerbose = 64,
     kVerbosityVeryVerbose = 128,
     kVerbosityDebug = 256,
   } verbosity_t;

   typedef enum {
     kOutputNormal = 1,
     kOutputRaw = 2,
     kOutputPlain = 4
   } output_t;

 public:

   // Writes a message to the output.
   virtual void write(const std::string &messages, 
              bool newline = false, uint16_t options = 0) = 0;

   // Writes a message to the output and adds a newline at the end.
   virtual void write_ln(const std::string &messages, uint16_t options = 0) = 0;

 public:

   // Sets the verbosity of the output.
   void set_verbosity(uint16_t level) {
     verbosity_ = level;
   }

   // Gets the current verbosity of the output.
   uint16_t get_verbosity() const {
     return verbosity_;
   }

   // Returns whether verbosity is quiet (-q).
   bool is_quiet() const {
     return kVerbosityQuiet == (kVerbosityQuiet & verbosity_);
   }

   // Returns whether verbosity is verbose (-v).
   bool is_verbose() const {
     return kVerbosityVerbose == (kVerbosityVerbose & verbosity_);
   }

   // Returns whether verbosity is very verbose (-vv).
   bool is_very_verbose() const {
     return kVerbosityVeryVerbose == (kVerbosityVeryVerbose & verbosity_);
   }

   // Returns whether verbosity is debug (-vvv).
   bool is_debug() const {
     return kVerbosityDebug == (kVerbosityDebug & verbosity_);
   }

   // Sets the decorated flag.
   void set_decorated(bool flag) {
     decorated_ = flag;
   }

   // Gets the decorated flag.
   bool get_decorated() const {
     return decorated_;
   }
   
 protected:

   uint16_t verbosity_;
   bool decorated_;

};

} // namespace console

} // namespace pf_interfaces

#endif // PF_INTERFACES_CONSOLE_OUTPUT_H_
