/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id command.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/03/05 10:07
 * @uses The console command class.
 */
#ifndef PF_CONSOLE_COMMAND_H_
#define PF_CONSOLE_COMMAND_H_

#include "pf/console/config.h"
#include "pf/console/input_definition.h"
#include "pf/console/input.h"
#include "pf/console/output.h"

namespace pf_console {

class Command {

 public:
   Command(const std::string &_name = "") {
     std::unique_ptr<InputDefinition> temp(new InputDefinition());
     definition_ = std::move(temp);
     std::string name{_name};
     if (name == "") name = get_default_name();
     set_name(name);
     // configure(); // Can't call virtual function in construct.
   }
   virtual ~Command() {}

 public:

  using code_t = std::function<uint8_t (Input *, Output *)>;

 public:

   typedef enum {
     kSuccess = 0,
     kFailure = 1
   } result_t;

 public:

   static std::string get_default_name() {
     return default_name_;
   }

 public:
   // Checks whether the command is enabled or not in the current environment.
   // 
   // Override this to check for x or y and return false if the command can not
   // run properly under the current conditions.
   virtual bool is_enabled() const {
     return true;
   }

   virtual void set_command(Command *) {}

   // Configures the current command.
   virtual void configure() {}

   // If parse tokens when bind input definition.
   virtual bool is_parse_input() const {
     return true;
   }

 public:

   // Ignores validation errors.
   void ignore_validation_errors() {
     ignore_validation_errors_ = true;
   }
   
   // Runs the command.
   //
   // The code to execute is either defined directly with the
   // set_code() method or by overriding the execute() method
   // in a sub-class.
   uint8_t run(Input *input, Output *output);

   // Sets the code to execute when running this command.
   //
   // If this method is used, it overrides the code defined 
   // in the execute() method.
   Command &set_code(code_t code) {
     code_ = code;
     return *this;
   }

   // Sets an array of argument and option instances.
   Command &set_definition(InputDefinition *definition) {
     std::unique_ptr<InputDefinition> pointer(definition);
     definition_ = std::move(pointer);
     return *this;
   }

   // Gets the InputDefinition attached to this Command.
   InputDefinition *get_definition() {
     return is_null(full_definition_) ? 
       definition_.get() : full_definition_.get();
   }

   // Adds an argument.
   // @throws std::invalid_argument When argument mode is not valid
   Command &add_argument(const std::string &name, 
       uint8_t mode = 0, 
       const std::string &description = "", const std::string &def = "") {
     definition_->add_argument(InputArgument(name, mode, description, def));
     return *this;
   }

   // Adds an option.
   // @throws std::invalid_argument If option mode is invalid or incompatible 
   Command &add_option(const std::string &name, 
       const std::string &shortcut = "",
       uint8_t mode = 0,
       const std::string &description = "", const std::string &def = "") {
     definition_->add_option(
         InputOption(name, shortcut, mode, description, def));
     return *this;
   }

   // Sets the name of the command.
   //
   // This method can set both the namespace and the name if
   // you separate them by a colon (:)
   // command->set_name('foo:bar');
   //
   // @throws std::invalid_argument When argument mode is not valid 
   Command &set_name(const std::string &name) {
     name_ =  name;
     return *this;
   }

   // Sets the process title of the command.
   //
   // This feature should be used only when creating a long process command, 
   // like a daemon.
   Command &set_process_title(const std::string &title) {
     process_title_ = title;
     return *this;
   }

   // Returns the command name.
   std::string name() const {
     return name_;
   }

   // Set hidden.
   Command &set_hidden(bool hidden = true) {
     hidden_ = hidden;
     return *this;
   }

   // Is hidden.
   bool is_hidden() const {
     return hidden_;
   }

   // Sets the description for the command. 
   Command &set_description(const std::string &description) {
     description_ = description;
     return *this;
   }

   // Returns the description for the command.
   std::string get_description() const {
     return description_;
   }

   // Sets the aliases for the command. 
   Command &set_aliases(const std::vector<std::string> &aliases) {
     for (const auto &aliase: aliases) {
       validate_name(aliase);
     }
     aliases_ = aliases;
     return *this;
   }

   // Returns the aliases for the command.
   std::vector<std::string> get_aliases() const {
     return aliases_;
   }

   // Returns the synopsis for the command.
   std::string get_synopsis(bool is_short = false) {
     std::string key = is_short ? "short" : "long";
     if (synopsis_.find(key) == synopsis_.end()) {
       synopsis_[key] = name_ + " " + definition_->get_synopsis(is_short);
     }
     return synopsis_[key];
   }

   // Add a command usage example, it'll be prefixed with the command name.
   Command &add_usage(const std::string &_usage) {
     std::string usage{_usage};
     if (usage.find(name_) != 0) {
       usage = name_ + " " + _usage;
     }
     usages_.emplace_back(usage);
     return *this;
   }

   // Returns alternative usages of the command. 
   std::vector<std::string> get_usages() const {
     return usages_;
   }

   // Returns the help string.
   std::string help() const {
     return help_;
   }

   // Sets the help string.
   void set_help(const std::string &str) {
     help_ = str;
   }

   // Sets the application.
   void set_application(Application *app) {
     app_ = app;
   }

   // Gets the application.
   Application *get_application() {
     return app_;
   }

   // Merges the application definition with the command definition.
   //
   // This method is not part of public API and should not be used directly.
   void merge_application_definition(bool merge_args = true);

 protected:

   static std::string default_name_;

 protected:

   // Executes the current command.Executes the current command.
   //
   // This method is not abstract because you can use this class 
   // as a concrete class. In this case, instead of defining the
   // execute() method, you set the code to execute by passing
   // a Closure to the setCode() method.
   virtual uint8_t execute(Input *input, Output *output) {
     std::cout << "execute: in virtual=========================" << std::endl;
     throw std::logic_error("You must override the execute() "
       "method in the concrete command class.");
     return 1;
   }

   // Interacts with the user.
   //
   // This method is executed before the InputDefinition is validated. 
   // This means that this is the only place where the command can
   // interactively ask for values of missing required arguments.
   virtual void interact(Input *, Output *) {}

   // Initializes the command after the input has been bound 
   // and before the input is validated.
   //
   // This is mainly useful when a lot of commands extends one main command 
   // where some things need to be initialized based on the input arguments 
   // and options.
   virtual void initialize(Input *, Output *) {}

 private:

   std::string name_;
   std::string process_title_;
   std::vector<std::string> aliases_;
   std::unique_ptr<InputDefinition> definition_;
   bool hidden_;
   std::string help_;
   std::string description_;
   std::unique_ptr<InputDefinition> full_definition_;
   bool ignore_validation_errors_;
   code_t code_;
   std::map<std::string, std::string> synopsis_;
   std::vector<std::string> usages_;
   Application *app_;

 private:

   // Validates a command name.
   //
   // It must be non-empty and parts can optionally be separated by ":".
   //
   // @throws std::invalid_argument When the name is invalid
   void validate_name(const std::string &name) {
     std::regex reg("^[^\\:]++(\\:[^\\:]++)*$");
     if (!std::regex_match(name, reg)) {
       std::string e = "Command name \"" + name + "\" is invalid.";
       throw std::invalid_argument(e);
     }
   }

};

} // namespace pf_console

#endif // PF_CONSOLE_COMMAND_H_
