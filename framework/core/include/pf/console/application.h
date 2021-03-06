/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id application.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/03/13 16:26
 * @uses An Application is the container for a collection of commands.
 *
 *       It is the main entry point of a Console application.
 *
 *       This class is optimized for a standard CLI environment.
 *
 *       auto app = new Application('myapp', '1.0 (stable)');
 *       app->add(new SimpleCommand());
 *       app->run();
 */

#ifndef PF_CONSOLE_APPLICATION_H_
#define PF_CONSOLE_APPLICATION_H_

#include "pf/console/config.h"
#include "pf/console/command.h"

namespace pf_console {

class Application {

 public:
   Application(const std::string &name = "UNKNOWN",
       const std::string &version = "UNKNOWN") {
     name_ = name;
     version_ = version;
     default_command_name_ = "list";
     single_command_ = false;
     initialized_ = false;
     want_helps_ = false;
     running_command_ = nullptr;
   }
   ~Application() {}

 public:

   void reset() {}

   // Gets the name of the application. 
   std::string get_name() const {
     return name_;
   }

   // Sets the application name.
   void set_name(const std::string &name) {
     name_ = name;
   }

   // Get command real name(from name or short name, if not exists return "").
   std::string get_command_real_name(const std::string &name) {
     std::string r{name};
     if (commands_.find(name) == commands_.end()) {
       if (command_aliases_.find(name) != command_aliases_.end()) {
         r = command_aliases_[name];  
       } else {
         r = "";
       }
     }
     return r;
   }

   // Gets the application version.
   std::string get_version() const {
     return version_;
   }

   // Sets the application version.
   void set_version(const std::string &version) {
     version_ = version;
   }

   // Gets the InputDefinition related to this Application.
   //
   // Not safe in multi threads(when use single command).
   InputDefinition *get_definition();

   void set_definition(InputDefinition *definition) {
     std::unique_ptr<InputDefinition> temp(definition);
     definition_ = std::move(temp);
   }

   // Runs the current application.
   uint8_t run(Input *input = nullptr, Output *output = nullptr);

   // Runs the current application.
   // @Returns 0 if everything went fine, or an error code
   uint8_t do_run(Input *input, Output *output);

   // Returns the long version of the application.
   std::string get_long_version() const;

   // Adds a command object.
   //
   // If a command with the same name already exists, it will be overridden.
   // If the command is not enabled it will not be added.
   Command *add(Command *command);

   // Adds an array of command objects.
   //
   // If a Command is not enabled it will not be added.
   void add_commands(std::vector<Command *> commands);

   // Returns a registered command by name or alias.
   Command *get(const std::string &name);

   // Gets the help message.
   std::string get_help() const {
     return get_long_version();
   }

   // Finds a registered namespace by a name or an abbreviation.
   std::string find_namespace(const std::string &_namespace);

   // Finds a command by name or alias.
   //
   // Contrary to get, this command tries to find the best
   // match if you give it an abbreviation of a name or alias.
   Command *find(const std::string &name);

   // Returns true if the command exists, false otherwise. 
   bool has(const std::string &name) {
     return get_command_real_name(name) != "";
   }

   // Gets the commands (registered in the given namespace if provided). 
   //
   // The array keys are the full names and the values the command pointers. 
   std::map<std::string, Command *> all(const std::string &_namespace = "");

   // Returns the namespace part of the command name.
   //
   // This method is not part of public API and should not be used directly.
   std::string extract_namespace(
       const std::string &name, int32_t limit = 0) const;

   // Sets the default Command name. 
   Application &set_default_command(
       const std::string &name, bool is_single_command);

   // Returns an array of all unique namespaces used by currently 
   // registered commands.
   std::vector<std::string> get_namespaces();

 protected:

   // Runs the current command.
   //
   // If an event dispatcher has been attached to the application,
   // events are also dispatched during the life-cycle of the command.
   uint8_t do_runcommand(Command *command, Input *input, Output *output);

   // Gets the name of the command based on input.
   std::string get_command_name(Input *input) const;

   // Gets the default input definition.
   InputDefinition get_default_input_definition() const;

   // Gets the default commands that should always be available.
   std::vector<Command *> get_default_commands() const;

   // Configures the input and output instances based on the 
   // user arguments and options.
   void configure_IO(Input *input, Output *output);

   bool is_single_command() const {
     return single_command_;
   }

 private:

   // Returns abbreviated suggestions in string format.
   std::string get_abbreviation_suggestions(
       const std::vector<std::string> &abbrevs) const;

   // Finds alternative of name among collection,
   // if nothing is found in collection, try in abbrevs.
   std::vector<std::string> find_alternatives(const std::string &name, 
       const std::vector<std::string> &collection) const;

   // Init.
   void init();

   // Returns all namespaces of the command name.
   std::vector<std::string> extract_all_namespace(const std::string &name);

 private:

   std::string name_;
   std::string version_;
   std::string default_command_name_;
   std::unique_ptr<InputDefinition> definition_;
   std::unique_ptr<InputDefinition> definition_temp_;
   bool single_command_;
   bool initialized_;
   bool want_helps_;
   std::map<std::string, std::unique_ptr<Command> > commands_;
   std::map<std::string, std::string> command_aliases_;
   Command *running_command_;

};

} // namespace pf_console

#endif // PF_CONSOLE_APPLICATION_H_
