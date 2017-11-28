#include "pf/basic/time_manager.h"
#include "pf/sys/assert.h"
#include "pf/net/connection/manager/basic.h"

using namespace pf_net::connection::manager;

bool Basic::heartbeat(uint32_t time) {
  using namespace pf_net;
  auto _time = 0 == time ? TIME_MANAGER_POINTER->get_tickcount() : time;
  auto _size = size();
  for (decltype(_size)i = 0; i < _size; ++i) {
    if (ID_INVALID == connection_idset_[i]) continue;
    connection::Basic* connection = nullptr;
    connection = pool_->get(connection_idset_[i]);
    if (nullptr == connection) {
      Assert(false);
      return false;
    }
    if (!connection->heartbeat(_time)) {
      remove(connection);
      Assert(false);
    }
  }
  return true;
}

void Basic::tick() {
  bool result = false;
  //normal.
  try {
    result = select();
    Assert(result);

    result = process_exception();
    Assert(result);

    result = process_input();
    Assert(result);

    result = process_output();
    Assert(result); 
  } catch(...) {
    
  }

  //command.
  try {
    result = process_command();
    Assert(result);
  } catch(...) {
  
  }
  //cache command. 
  try {
    result = process_command_cache();
    Assert(result);
  } catch(...) {

  }

  //heartbeat.
  try {
    result = heartbeat();
    Assert(result);
  } catch(...) {

  }
}
