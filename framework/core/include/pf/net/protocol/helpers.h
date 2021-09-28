/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id helpers.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/27 17:38
 * @uses The net protocol helpers.
 */

#ifndef PF_NET_PROTOCOL_HELPERS_H_
#define PF_NET_PROTOCOL_HELPERS_H_

#include "pf/net/protocol/config.h"
#include "pf/net/protocol/basic.h"
#include "pf/net/protocol/standard.h"

namespace pf_net {

namespace protocol {

static inline std::map<std::string, Interface *> &get_all() {
  static std::map<std::string, Interface *> all;
  static bool initialized{false};
  if (!initialized) {
    static Basic basic;
    all["default"] = &basic;
    static Standard standard;
    all["standard"] = &standard;
    initialized = true;
  }
  return all;
}

static inline void set(const std::string &name, Interface *pointer) {
  auto all = get_all();
  all[name] = pointer;
}

static inline Interface *get(const std::string &name) {
  auto all = get_all();
  if (all.find(name) == all.end()) return nullptr;
  return all[name];
}

} // namespace protocol

} // namespace pf_net

#endif // PF_NET_PROTOCOL_HELPERS_H_
