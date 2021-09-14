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
std::enable_if<detail::is_bounded_array_v<T>> make_unique(Args&&...) = delete;

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

template< class T >
using remove_const_t = typename std::remove_const<T>::type;

} // namespace pf_basic

#endif // PF_BASIC_TRAITS_H_
