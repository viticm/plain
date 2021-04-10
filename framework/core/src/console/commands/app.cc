#include "pf/net/connection/manager/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/manager/listener_factory.h"
#include "pf/engine/application.h"
#include "pf/engine/kernel.h"
#include "pf/console/input_definition.h"
#include "pf/console/application.h"
#include "pf/console/commands/app.h"

using namespace pf_console;
using namespace pf_interfaces::console;
using namespace pf_console::commands;

static void status_info(pf_console::Output *output) {
  if (is_null(ENGINE_POINTER)) return;
  std::string str;

  // The default net info.
  str = "default net->";
  auto def_net = ENGINE_POINTER->get_net();
  str += " active:";
  if (is_null(def_net)) {
    str += " 0 count: 0 max count: 0";
  } else {
    str += " 1 count: " + std::to_string(def_net->size());
    str += " max count: " + std::to_string(def_net->max_size());
  }
  output->write_ln(str);

  // The services.
  auto net_factory = ENGINE_POINTER->get_net_listener_factory(false);
  if (net_factory) {
    str = "Extra service: " + std::to_string(net_factory->size());
    output->write_ln(str);
    auto listen_list = ENGINE_POINTER->get_listen_list();
    for (auto it = listen_list.begin(); it != listen_list.end(); ++it) {
      str = "  name: " + it->first;
      auto net = net_factory->getenv(it->second);
      if (!is_null(net)) {
        str += " count: " + std::to_string(net->size());
        str += " max count: " + std::to_string(net->max_size()); 
      } else {
        str += " not active";
      }
      output->write_ln(str);
    }
  } else {
    output->write_ln("Extra service: 0");
  }
}

void App::configure() {
  std::vector<InputParameter *> p;
  std::unique_ptr<InputParameter> p1(new InputArgument(
        "namespace", InputParameter::kModeOptional, 
        "The namespace name", "app"
        ));
  p.emplace_back(p1.get());
  std::unique_ptr<InputParameter> p2(new InputOption(
        "raw", "", InputParameter::kModeNone, "To output raw command help", ""
        ));
  p.emplace_back(p2.get());

  std::unique_ptr<InputParameter> p3(new InputOption(
        "stop", "k", InputParameter::kModeNone, "Stop the application", "-1"
        ));
  p.emplace_back(p3.get());

  std::unique_ptr<InputParameter> p4(new InputOption(
        "status", "s", InputParameter::kModeNone, 
        "Get the application status info", "-1"
        ));
  p.emplace_back(p4.get());
  
  auto definition = new InputDefinition(p);
  set_name("app")
    .set_definition(definition)
    .set_description("Apps commands")
    .set_help("App the all can use command.");
}

uint8_t App::execute(Input *input, Output *output) {
  // std::cout << "App::execute" << std::endl;
  auto kernel = pf_engine::Application::getsingleton_pointer();
  if (is_null(kernel)) return kCommandExecuteInvalid;
  auto stop = input->get_option("stop");
  if (stop != "-1") {
    output->write_ln("Now stop application.");
    kernel->stop();
    return kCommandExecuteNone;
  }
  auto status = input->get_option("status");
  if (status != "-1") {
    status_info(output);
  }
  return kCommandExecuteNone;
}
