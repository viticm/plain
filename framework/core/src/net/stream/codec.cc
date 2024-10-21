#include "plain/net/stream/codec.h"
#include "plain/net/packet/basic.h"
#include "plain/net/stream/basic.h"

namespace plain::net::stream {

using length_t = uint32_t;
#pragma pack(push,1) // The net buffer head can't alignas with system.
struct head_t {
  packet::id_t id;
  length_t length;
};
#pragma pack(pop)
static constexpr size_t kHeaderSize{sizeof(head_t)};

bytes_t encode(std::shared_ptr<packet::Basic> packet) {
  bytes_t r;
  auto d = packet->data();
  head_t head;
  head.id = hton(packet->id());
  head.length = hton(static_cast<length_t>(d.size()));
  r.append(reinterpret_cast<std::byte *>(&head), sizeof(head));
  r.append(d.data(), d.size());
  return r;
}

error_or_t<std::shared_ptr<packet::Basic>>
decode(Basic *input, const packet::limit_t &packet_limit) {
  if (!input) {
    return Error{ErrorCode::RunTime, ""};
  }
  head_t head;
  auto head_pointer = reinterpret_cast<std::byte *>(&head);
  memset(head_pointer, 0, kHeaderSize);
  if (input->peek(head_pointer, kHeaderSize) != kHeaderSize) {
    return Error{ErrorCode::NetPacketCantFill, ""};    
  }
  head.id = ntoh(head.id);
  head.length = ntoh(head.length);
  // std::cout << "decode: " << head.id << "|" << head.length << std::endl;
  if (head.id == 0 || head.id > packet_limit.max_id ||
      head.length > packet_limit.max_length) {
    return Error{ErrorCode::NetPacketInvalid, ""};
  }
  if (head.length + kHeaderSize > input->size()) {
    return Error{ErrorCode::NetPacketNeedRecv, ""};
  }
  auto p = std::make_shared<packet::Basic>();
  p->set_id(head.id);
  p->set_writeable(true);
  bytes_t bytes;
  bytes.resize(head.length);
  input->remove(kHeaderSize);
  auto size = input->read(bytes.data(), head.length);
  // std::cout << "read: " << size << "|" << (char *)bytes.data() << std::endl;
  if (size != head.length) {
    return Error{ErrorCode::RunTime, ""};
  }
  p->write(bytes.data(), bytes.size());
  p->set_writeable(false);
  p->set_readable(true);
  if (head.id == packet::kRpcRequestId) {
    p->set_call_request(true);
  } else if (head.id == packet::kRpcResponseId) {
    p->set_call_response(true);
  } else if (head.id == packet::kRpcNotifyId) {
    p->set_call_notify(true);
  }
  return p;
}

// For rpc.
static constexpr plain::bytes_t
kRpcRequestBegin{std::byte{0x63}, std::byte{0x72}};
static constexpr plain::bytes_t
kRpcRequestEnd{std::byte{0x72}, std::byte{0x63}};
static constexpr plain::bytes_t
kRpcResponseBegin{std::byte{0x63}, std::byte{0x6f}};
static constexpr plain::bytes_t
kRpcResponseEnd{std::byte{0x6f}, std::byte{0x63}};
static constexpr plain::bytes_t
kRpcNotifyBegin{std::byte{0x63}, std::byte{0x6e}};
static constexpr plain::bytes_t
kRpcNotifyEnd{std::byte{0x6e}, std::byte{0x63}};

static constexpr size_t kRpcRequestSizeMin{4 + 4 + 1};
static constexpr size_t kRpcNotifySizeMin{4 + 1};
static constexpr size_t kRpcResponseSizeMin{4 + 4 + 4 + 1};

error_or_t<std::shared_ptr<packet::Basic>>
line_decode(stream::Basic *input, const packet::limit_t &packet_limit) {
  if (!input) return Error{ErrorCode::RunTime};
  bytes_t bytes;
  bytes.reserve(packet_limit.max_length);
  auto readed = input->peek(bytes.data(), bytes.capacity());
  if (readed == 0) return ErrorCode{ErrorCode::NetPacketNeedRecv};
  std::string_view str{reinterpret_cast<char *>(bytes.data()), readed};
  auto pos = str.find('\n');
  if (pos == std::string::npos) {
    if (readed == packet_limit.max_length )
      return Error{ErrorCode::RunTime};
    else
      return Error{ErrorCode::NetPacketNeedRecv};
  }
  input->remove(pos + 1); // remove readed line.
  if (pos > 0 && str[pos - 1] == '\r') {
    pos -= 1;
  }
  auto p = std::make_shared<packet::Basic>();

  // For rpc
  if (pos >= kRpcRequestSizeMin &&
    bytes_t{bytes.data(), 2} == kRpcRequestBegin &&
    bytes_t{bytes.data() + pos - 2, 2} == kRpcRequestEnd) {
    p->set_call_request(true);
  } else if (pos >= kRpcResponseSizeMin &&
    bytes_t{bytes.data(), 2} == kRpcResponseBegin &&
    bytes_t{bytes.data() + pos - 2, 2} == kRpcResponseEnd) {
    p->set_call_response(true);
  } else if (pos >= kRpcNotifySizeMin &&
    bytes_t{bytes.data(), 2} == kRpcNotifyBegin &&
    bytes_t{bytes.data() + pos - 2, 2} == kRpcNotifyEnd) {
    p->set_call_notify(true);
  }

  if (pos > 0) {
    p->set_writeable(true);
    if (p->is_call_request() || p->is_call_response() || p->is_call_notify()) { // Rpc
      p->write(bytes.data() + 2, pos - 2);
    } else {
      p->write(bytes.data(), pos);
    }
    p->set_writeable(false);
  }
  p->set_readable(true);
  return p;
}

bytes_t line_encode(std::shared_ptr<packet::Basic> packet) {
  auto d = packet->data();
  bytes_t r{d.data(), d.size()};

  // For rpc
  if (packet->is_call_request()) {
    r = kRpcRequestBegin + r + kRpcRequestEnd;
  } else if (packet->is_call_response()) {
    r = kRpcResponseBegin + r + kRpcResponseEnd;
  } else if (packet->is_call_notify()) {
    r = kRpcNotifyBegin + r + kRpcNotifyEnd;
  }

  r.push_back(std::byte{'\n'});
  return r;
}

} // namespace plain::net::stream
