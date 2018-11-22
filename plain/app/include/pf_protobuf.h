/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id pf_protobuf.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/07/21 13:38
 * @uses your description
*/
#ifndef PF_PROTOBUF_H_
#define PF_PROTOBUF_H_

#include "pf_plugin/protocol.h"

/* Cast to normal packet pointer. */
#define normalpack(p) dynamic_cast<pf_plugin::protocol::NormalPacket *>(p)

/* Cast to logic packet pointer. */
#define logicpack(p) dynamic_cast<pf_plugin::protocol::LogicPacket *>(p)

/* Cast to large packet pointer. */
#define largepack(p) dynamic_cast<pf_plugin::protocol::LargePacket *>(p)


/* Send normal protobuf packet.
 * conn: The connection object.
 * id: The packet id.
 * s: The protobuf struct object.
 * */
#define protobuf_send(conn,id,s) { \
  pf_plugin::protocol::NormalPacket p(id); \
  std::string str{""}; s.SerializeToString(&str); \
  p.set_str(str); \
  (conn).send(&p); \
}

/* Send normal protobuf large packet.
 * conn: The connection object.
 * id: The packet id.
 * s: The protobuf struct object.
 * */
#define protobuf_lgsend(conn,id,s) { \
  pf_plugin::protocol::LargePacket p(id); \
  std::string str{""}; s.SerializeToString(&str); \
  p.set_str(str); \
  (conn).send(&p); \
}

/* Send logic protobuf packet.
 * conn: The connection object.
 * id: The packet id.
 * lid: logic id.
 * op: option.
 * s: The protobuf struct object.
 * */
#define protobuf_lsend(conn,id,lid,op,s) { \
  pf_plugin::protocol::LogicPacket p(id); \
  p.set_logic_id(lid); \
  p.set_option(op); \
  std::string str{""}; s.SerializeToString(&str); \
  p.set_str(str); \
  (conn).send(&p); \
}

/* Get protobuf data from packet. */
#define protobuf_get(p,s) { \
  auto pack = normalpack(p); \
  s.ParseFromString(pack->get_str()); \
}

/* Get protobuf data from logic packet. */
#define protobuf_lget(p,fid,cid,s) { \
  auto pack = logicpack(p); \
  fid = p->get_parent_id(); \
  cid = p->get_child_Id(); \
  s.ParseFromString(p.get_str()); \
}

/* Get protobuf data from large packet. */
#define protobuf_lgget(p,s) { \
  auto pack = largepack(p); \
  s.ParseFromString(pack->get_str()); \
}

#endif //PF_PROTOBUF_H_
