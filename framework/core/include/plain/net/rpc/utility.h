/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id utility.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/09/06 18:11
 * @uses The net rpc utility implemention.
 */

#ifndef PLAIN_NET_RPC_UTILITY_H_
#define PLAIN_NET_RPC_UTILITY_H_

#include "plain/net/rpc/config.h"
#include "plain/net/rpc/packer.h"
#include "plain/net/rpc/unpacker.h"

namespace plain::net {
namespace rpc {

template <typename T>
std::vector<uint8_t> pack(T &obj) noexcept {
  auto packer = Packer{};
  obj.pack(packer);
  return packer.vector();
}

template <typename T>
std::vector<uint8_t> pack(T &&obj) noexcept {
  auto packer = Packer{};
  obj.pack(packer);
  return packer.vector();
}

template <typename T>
T unpack(const uint8_t *data, const size_t size, Error &error) noexcept {
  auto obj = T{};
  auto unpacker = Unpacker(data, size);
  obj.pack(unpacker);
  error = unpacker.error_;
  return obj;
}

template <typename T>
T unpack(const uint8_t *data, const size_t size) noexcept {
  Error error;
  return unpack<T>(data, size, error);
}

template <typename T>
T unpack(const std::vector<uint8_t> &data, Error &error) noexcept {
  return unpack<T>(data.data(), data.size(), error);
}

template <typename T>
T unpack(const std::vector<uint8_t> &data) {
  Error error;
  return unpack<T>(data.data(), data.size(), error);
}

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_UTILITY_H_
