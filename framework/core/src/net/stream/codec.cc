#include "plain/net/stream/codec.h"
#include "plain/net/packet/basic.h"
#include "plain/net/stream/basic.h"

namespace plain::net::stream {

using length_t = uint32_t;
struct head_t {
  packet::id_t id;
  length_t length;
};
static constexpr size_t kHeaderSize{sizeof(head_t)};

error_or_t<std::shared_ptr<packet::Basic>> decode(Basic *input) {
  if (!input) {
    return Error{ErrorCode::RunTime, ""};
  }
  head_t head;
  auto head_pointer = reinterpret_cast<std::byte *>(&head);
  memset(head_pointer, 0, kHeaderSize);
  if (!input->peek(head_pointer, kHeaderSize)) {
    return Error{ErrorCode::NetPacketCantFill, ""};    
  }
  head.id = ntoh(head.id);
  head.length = ntoh(head.length);
  if (head.length + kHeaderSize > input->size()) {
    return Error{ErrorCode::NetPacketNeedRecv, ""};
  }
  auto p = std::make_shared<packet::Basic>();
  p->set_id(head.id);
  p->set_writeable(true);
  bytes_t bytes;
  bytes.reserve(head.length);
  input->read(bytes);
  if (bytes.size() != head.length) {
    return Error{ErrorCode::RunTime, ""};
  }
  p->write(bytes);
  p->set_writeable(false);
  p->set_readable(true);
  return p;
}

bytes_t encode(std::shared_ptr<packet::Basic> packet) {
  packet::id_t id = packet->id();
  bytes_t r;
  id = hton(id);
  r.append(reinterpret_cast<std::byte *>(&id), sizeof(id));
  auto d = packet->data();
  length_t length = d.size();
  length = hton(length);
  r.append(reinterpret_cast<std::byte *>(&length), sizeof(length));
  r.append(d.data(), d.size());
  return r;
}

} // namespace plain::net::stream
