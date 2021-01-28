/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id input_parameter.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/01/04 13:31
 * @uses The console input parameter interface.
 */
#ifndef PF_INTERFACES_CONSOLE_INPUT_PARAMETER_H_
#define PF_INTERFACES_CONSOLE_INPUT_PARAMETER_H_

#include "pf/interfaces/console/config.h"
#include "pf/sys/assert.h"

namespace pf_interfaces {

namespace console {

class InputParameter {

 public:

   typedef enum {
     kModeNone = 1,
     kModeRequired = 2,
     kModeOptional = 4,
     kModeIsAarray = 8,
   } mode_t;

 public:

   InputParameter() {}
   
   InputParameter(const std::string &name, 
                  mode_t mode = kModeNone, 
                  const std::string &description = "",
                  const std::string &def = "") {
     if (kModeNone == mode) {
       mode = kModeOptional;
     } else if (mode > 15 || mode < 1) {
       char msg[128]{0};
       snprintf(msg, sizeof(msg) - 1, "The parameter mode[%d] invalid", mode);
       AssertEx(false, msg);
     }
     name_ = name;
     mode_ = mode;
     description_ = description;
     set_default(def);
   }

   InputParameter(const std::string &name, 
                  mode_t mode = kModeNone, 
                  const std::string &description = "",
                  const std::vector<std::string> &def = {}) {
     if (kModeNone == mode) {
       mode = kModeOptional;
     } else if (mode > 15 || mode < 1) {
       char msg[128]{0};
       snprintf(msg, sizeof(msg) - 1, "The parameter mode[%d] invalid", mode);
       AssertEx(false, msg);
     }
     name_ = name;
     mode_ = mode;
     description_ = description;
     set_defaults(def);
   }

   virtual ~InputParameter() {}

 public:

   //Returns the class name.
   virtual const std::string class_name() const {
     return "InputParameter";
   }

 public:
    
   //Is the parameter accept value.
   bool accept_value() const {
     return is_required() || is_optional();
   }

   //Returns the argument name.
   std::string name() const { return name_; }

   // Returns true if the argument is required.
   bool is_required() const {
     return kModeRequired == (kModeRequired & mode_);
   }

   // Returns true if the argument is optional.
   bool is_optional() const {
     return kModeOptional == (kModeOptional & mode_);
   }

   // Returns true if the argument can take multiple values.
   bool is_array() const {
     return kModeIsAarray == (kModeIsAarray & mode_);
   }

   // Returns the shortcut name.
   virtual std::string shortcut() const { return ""; }

   // Sets the default value.
   void set_default(const std::string &def = "") {
     if (kModeRequired == mode_ && "" != def) {
       AssertEx(false, "Cannot set a default value except for optional mode.");
     }
     if (this->is_array()) {
       AssertEx(false, 
           "A default value for an array argument must be an array.");
     }
     def_ = def;
   }

   // Sets the default value.
   void set_defaults(const std::vector<std::string> &defs = {}) {
     if (kModeRequired == mode_ && !defs.empty()) {
       AssertEx(false, "Cannot set a default value except for optional mode.");
     }
     defs_ = defs;
   }

   // Returns the default value.
   std::string get_default() const {
     return def_;
   }

   // Returns the defaults value.
   std::vector<std::string> get_defaults() const {
     return defs_;
   }

   // Returns the description text.
   std::string get_description() const {
     return description_;
   }

   // Checks whether the given parameter equals this one.
   bool equals(InputParameter *parameter) const {
     return parameter->name() == this->name()
         && parameter->shortcut() == this->shortcut()
         && parameter->get_default() == this->get_default()
         && parameter->get_defaults() == this->get_defaults()
         && parameter->is_array() == this->is_array()
         && parameter->is_required() == this->is_required()
         && parameter->is_optional() == this->is_optional();
   }


 private:

   std::string name_;
   mode_t mode_;
   std::string def_;
   std::vector<std::string> defs_;
   std::string description_;

};

} // namespace console

} // namespace pf_interfaces

#endif //PF_INTERFACES_CONSOLE_INPUT_PARAMETER_H_
