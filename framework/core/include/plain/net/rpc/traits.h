/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id type_traits.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/09/05 11:31
 * @uses The type traits of rpc implemention.
 */

#ifndef PLAIN_NET_RPC_TYPE_TRAITS_H_
#define PLAIN_NET_RPC_TYPE_TRAITS_H_

#include "plain/net/rpc/config.h"
#include <list>
#include <set>
#include <map>
#include <unordered_map>

namespace plain::net {
namespace rpc {

template <typename T>
struct is_container {
  static constexpr bool value = false;
};

template <typename T>
inline static constexpr bool is_container_v = is_container<T>::value;

template <typename T, typename Alloc>
struct is_container<std::vector<T, Alloc>> {
  static constexpr bool value = true;
};

template <typename T, typename Alloc>
struct is_container<std::list<T, Alloc>> {
  static constexpr bool value = true;
};

template <typename T, typename Alloc>
struct is_container<std::set<T, Alloc>> {
  static constexpr bool value = true;
};

template <typename T, typename Alloc>
struct is_container<std::map<T, Alloc>> {
  static constexpr bool value = true;
};

template <typename T, typename Alloc>
struct is_container<std::unordered_map<T, Alloc>> {
  static constexpr bool value = true;
};

template <typename T>
struct is_stdarray {
  static constexpr bool value = false;
};

template <typename T, std::size_t N>
struct is_stdarray<std::array<T, N>> {
  static constexpr bool value = true;
};

template <typename T>
struct is_map {
  static constexpr bool value = false;
};

template <typename T, typename Alloc>
struct is_map<std::map<T, Alloc>> {
  static constexpr bool value = true;
};

template <typename T, typename Alloc>
struct is_map<std::unordered_map<T, Alloc>> {
  static constexpr bool value = true;
};

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_TYPE_TRAITS_H_
