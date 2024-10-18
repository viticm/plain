#include "plain/net/rpc/dispatcher.h"
#include "plain/basic/io.h"
#include "plain/net/packet/basic.h"

using plain::net::rpc::Dispatcher;

plain::error_or_t<std::shared_ptr<plain::net::rpc::Packer>>
Dispatcher::dispatch(std::shared_ptr<packet::Basic> pack) {
  
  std::string name;
  *pack >> name;
  if (name.empty()) return Error{ErrorCode::NetPacketInvalid};

  auto it = funcs_.find(name);
  if (it == funcs_.end()) return Error{ErrorCode::NetRpcFunctionNotFound};

  io_cdebug("rpc: [{}] called", name);

  auto data = reinterpret_cast<const uint8_t *>(
    pack->data().data() + pack->offset());
  auto size = pack->data().size() - pack->offset();
  Unpacker unpacker(data, size);

  return it->second(unpacker);
}

void Dispatcher::enforce_unique_name(const std::string &name) {
  auto it = funcs_.find(name);
  if (it != funcs_.end()) {
    throw std::logic_error(
      std::format(
        "Function name already bound: '{}'. "
        "Please use unique function names", name));
  }
}
