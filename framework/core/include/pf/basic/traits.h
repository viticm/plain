/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id traits.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/09/07 10:14
 * @uses ! This file include by basic config file default.
 */

#ifndef PF_BASIC_TRAITS_H_
#define PF_BASIC_TRAITS_H_

#include <cstdlib>
#include <type_traits>
#include <memory>

namespace pf_basic {

/*
// C++14 make_unique
namespace detail {

template<class>
constexpr bool is_unbounded_array_v = false;
template<class T>
constexpr bool is_unbounded_array_v<T[]> = true;
 
template<class>
constexpr bool is_bounded_array_v = false;
template<class T, size_t N>
constexpr bool is_bounded_array_v<T[N]> = true;

} // namespace detail

template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;
 
template<class T, class... Args>
enable_if_t<!std::is_array<T>::value, std::unique_ptr<T>>
make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
 
template<class T>
enable_if_t<detail::is_unbounded_array_v<T>, std::unique_ptr<T>>
make_unique(size_t n) {
    return std::unique_ptr<T>(new std::remove_extent<T>[n]());
}
 
template<class T, class... Args>
enable_if_t<detail::is_bounded_array_v<T>> make_unique(Args&&...) = delete;

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

template< class T >
using remove_const_t = typename std::remove_const<T>::type;
*/
namespace detail {

// helper to construct a non-array unique_ptr
template <typename T>
struct make_unique_helper {
  typedef std::unique_ptr<T> unique_ptr;

  template <typename... Args>
  static inline unique_ptr make(Args&&... args) {
    return unique_ptr(new T(std::forward<Args>(args)...));
  }
};

// helper to construct an array unique_ptr
template<typename T>
struct make_unique_helper<T[]> {
  typedef std::unique_ptr<T[]> unique_ptr;

  template <typename... Args>
  static inline unique_ptr make(Args&&... args) {
    return unique_ptr(new T[sizeof...(Args)]{std::forward<Args>(args)...});
}
};

// helper to construct an array unique_ptr with specified extent
template<typename T, std::size_t N>
struct make_unique_helper<T[N]> {
  typedef std::unique_ptr<T[]> unique_ptr;

  template <typename... Args>
  static inline unique_ptr make(Args&&... args) {
    static_assert(N >= sizeof...(Args),
        "For make_unique<T[N]> N must be as largs as the number of arguments");
    return unique_ptr(new T[N]{std::forward<Args>(args)...});
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ <= 6
  // G++ 4.6 has an ICE when you have no arguments
  static inline unique_ptr make() {
    return unique_ptr(new T[N]);
  }
#endif
};

} // namespace detail

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

template< class T >
using remove_const_t = typename std::remove_const<T>::type;

template <typename T, typename... Args>
inline typename detail::make_unique_helper<T>::unique_ptr
make_unique(Args&&... args) {
  return detail::make_unique_helper<T>::make(std::forward<Args>(args)...);
}

} // namespace pf_basic

#endif // PF_BASIC_TRAITS_H_
