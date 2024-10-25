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

template <typename T>
struct is_tuple {
  static constexpr bool value{false};
};

template <typename T>
struct is_tuple<std::tuple<T>> {
  static constexpr bool value{true};
};

template <typename ...Args>
struct is_tuple<std::tuple<Args...>> {
  static constexpr bool value{true};
};

template <typename T, T I>
struct constant : std::integral_constant<T, I> {};

template <bool B>
using bool_ = constant<bool, B>;

using true_ = bool_<true>;

using false_ = bool_<false>;

template <typename T>
using invoke = T::type;

template <int N>
using is_zero = invoke<std::conditional<(N == 0), true_, false_>>;

template <int N, typename... Ts>
using nth_type = invoke<std::tuple_element<N, std::tuple<Ts...>>>;

namespace tags {

// tags for the function traits, used for tag dispatching
struct zero_arg {};
struct nonzero_arg {};
struct void_result {};
struct nonvoid_result {};

template <int N> struct arg_count_trait { typedef nonzero_arg type; };

template <> struct arg_count_trait<0> { typedef zero_arg type; };

template <typename T> struct result_trait { typedef nonvoid_result type; };

template <> struct result_trait<void> { typedef void_result type; };
}

//! \brief Provides a small function traits implementation that
//! works with a reasonably large set of functors.
template <typename T>
struct func_traits : func_traits<decltype(&T::operator())> {};

template <typename C, typename R, typename... Args>
struct func_traits<R (C::*)(Args...)> : func_traits<R (*)(Args...)> {};

template <typename C, typename R, typename... Args>
struct func_traits<R (C::*)(Args...) const> : func_traits<R (*)(Args...)> {};

template <typename R, typename... Args> struct func_traits<R (*)(Args...)> {
  using result_type = R;
  using arg_count = std::integral_constant<std::size_t, sizeof...(Args)>;
  using args_type = std::tuple<typename std::decay<Args>::type...>;
};

template <typename T>
struct func_kind_info : func_kind_info<decltype(&T::operator())> {};

template <typename C, typename R, typename... Args>
struct func_kind_info<R (C::*)(Args...)> : func_kind_info<R (*)(Args...)> {};

template <typename C, typename R, typename... Args>
struct func_kind_info<R (C::*)(Args...) const>
  : func_kind_info<R (*)(Args...)> {};

template <typename R, typename... Args> struct func_kind_info<R (*)(Args...)> {
  typedef typename tags::arg_count_trait<sizeof...(Args)>::type args_kind;
  typedef typename tags::result_trait<R>::type result_kind;
};

template <typename F> using is_zero_arg = is_zero<func_traits<F>::arg_count>;

template <typename F>
using is_single_arg =
  invoke<std::conditional<func_traits<F>::arg_count == 1, true_, false_>>;

template <typename F>
using is_void_result = std::is_void<typename func_traits<F>::result_type>;


} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_TYPE_TRAITS_H_
