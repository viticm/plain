#include "pf/db/factory.h"
#include "pf/db/interface.h"
#include "pf/engine/application.h"
#include "pf/engine/kernel.h"
#include "pf/sys/thread.h"
#include "pf/net/connection/manager/basic.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/manager/connector.h"
#include "pf/net/connection/manager/listener_factory.h"
#include "pf/console/input_definition.h"
#include "pf/console/application.h"
#include "pf/console/commands/app.h"

using namespace pf_console;
using namespace pf_interfaces::console;
using namespace pf_console::commands;

static void status_info(pf_console::Output *output) {
  using namespace pf_net::connection::manager;
  if (is_null(ENGINE_POINTER)) return;

  output->write_ln("Current running threads: " + 
      std::to_string(pf_sys::ThreadCollect::count()));

  std::string str;

  // The default net info.
  str = "default net->";
  auto def_net = reinterpret_cast<Listener *>(ENGINE_POINTER->get_net());
  str += " active:";
  if (is_null(def_net)) {
    str += " 0";
  } else {
    str += " port: " + std::to_string(def_net->port());
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
        str += " port: " + std::to_string(net->port());
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

  // The clients.
  auto connect_list = ENGINE_POINTER->get_connect_list();
  str = "Connector: " + std::to_string(connect_list.size());
  output->write_ln(str);
  auto connector = ENGINE_POINTER->get_connector();
  if (!is_null(connector)) {
    for (auto it = connect_list.begin(); it != connect_list.end(); ++it) {
      str = "  name: " + it->first;
      auto connection = connector->get(static_cast<int16_t>(it->second));
      if (!is_null(connection) && !connection->is_disconnect()) {
        auto socket = connection->socket();
        str += " active: 1";
        str += " ip: ";
        str += socket->host();
        str += " port: " + std::to_string(socket->port());
      } else {
        str += " active: 0";
      }
    }
  }

  // Databases.
  if (GLOBALS["default.db.open"] == true) {
    str = "Default database active: 1 name: " + GLOBALS["default.db.name"].data;
    output->write_ln(str);
  } else {
    output->write_ln("Default database active: 0");
  }
  auto db_list = ENGINE_POINTER->get_db_list();
  output->write_ln("User database count: " + std::to_string(db_list.size()));
  auto db_factory = ENGINE_POINTER->get_db_factory();
  for (auto it = db_list.begin(); it != db_list.end(); ++it) {
    str = "  name: " + it->first;
    auto db = !is_null(db_factory) ? db_factory->getenv(it->second) : nullptr;
    str += " active: " + std::to_string(!is_null(db) ? 1 : 0);
    output->write_ln(str);
  }

  // Scripts.
  if (GLOBALS["default.script.open"] == true) {
    str = "Default script active: 1 type: " + 
      GLOBALS["default.script.type"].data;
    output->write_ln(str);
  } else {
    output->write_ln("Default script active: 0");
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
    .set_help(
        "App commands\n"
        " --stop|-k Stop the application\n"
        " --status|-s Get the application status info");
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
