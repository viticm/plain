/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id endian.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 18:15
 * @uses The plain endian endian_implemention.
 *       @refer: https://github.com/jmacheta/endian
 */
#ifndef PLAIN_BASIC_ENDIAN_H_
#define PLAIN_BASIC_ENDIAN_H_

#include "plain/basic/config.h"

namespace plain {

namespace endian_impl {

constexpr std::uint8_t
bswap(std::uint8_t n) noexcept {
  return n;
}

constexpr std::uint16_t
bswap(std::uint16_t n) noexcept {
  return static_cast<std::uint16_t>((n << 8U) | (n >> 8U));
}

constexpr std::uint32_t
bswap(std::uint32_t n) noexcept {
  return static_cast<std::uint32_t>(
      bswap(static_cast<std::uint16_t>(n))) << 16U |
      static_cast<std::uint32_t>(bswap(static_cast<std::uint16_t>(n >> 16U)));
}
    
constexpr std::uint64_t
bswap(std::uint64_t n) noexcept {
  return static_cast<std::uint64_t>(
      bswap(static_cast<std::uint32_t>(n))) << 32U |
      static_cast<std::uint64_t>(bswap(static_cast<std::uint32_t>(n >> 32U)));
}

template <typename T>
concept contiguous_byte_iterator = std::contiguous_iterator<T> &&
(1 == sizeof(typename std::iterator_traits<T>::value_type));

template <typename T>
concept contiguous_output_byte_iterator = contiguous_byte_iterator<T> &&
std::output_iterator<T, typename std::iterator_traits<T>::value_type>;

} // namespace endian_impl


template <std::integral T>
[[nodiscard]] constexpr T
byteswap(T n) noexcept {
#ifdef __cpp_lib_byteswap
  return std::byteswap(n);
#else
  if constexpr (1 == sizeof(n)) {
    return static_cast<T>(endian_impl::bswap(static_cast<std::uint8_t>(n)));
  } else if constexpr (2 == sizeof(n)) {
    return static_cast<T>(endian_impl::bswap(static_cast<std::uint16_t>(n)));
  } else if constexpr (4 == sizeof(n)) {
    return static_cast<T>(endian_impl::bswap(static_cast<std::uint32_t>(n)));
  } else if constexpr (8 == sizeof(n)) {
    return static_cast<T>(endian_impl::bswap(static_cast<std::uint64_t>(n)));
  } else {
    static_assert(0 == sizeof(n), 
        "ecpp::byteswap is not endian_implemented for type of this size");
  }
#endif
}

template <std::integral T> 
[[nodiscard]] constexpr T
ntoh(T n) noexcept {
  return (std::endian::native == std::endian::big) ? n : byteswap(n);
}

template <std::integral T, endian_impl::contiguous_byte_iterator CByteIt>
[[nodiscard]] constexpr T
ntoh(CByteIt src) noexcept {
  using value_type = std::remove_cvref_t<T>;

#ifdef __cpp_lib_bit_cast
  // Thanks to constexpr bit_cast we can evaluate this expression during 
  // compilation

  using iterator_value_type = std::remove_cvref_t<
    typename std::iterator_traits<CByteIt>::value_type>;
  struct {
    alignas(value_type) iterator_value_type value[sizeof(value_type)];
  } tmp;
  std::copy_n(src, sizeof(value_type), std::begin(tmp.value));
  auto value = std::bit_cast<value_type>(tmp);
#else
  value_type value;
  std::memcpy(&value, &*src, sizeof(value)); // Because of memcpy, 
                                             // this expression will NOT be 
                                             // constant expression
#endif
  return (std::endian::native == std::endian::big) ? value : byteswap(value);
}
template <std::integral T, endian_impl::contiguous_output_byte_iterator iter_t>
constexpr void 
ntoh(T n, iter_t dest) noexcept {
  using value_type = std::remove_cvref_t<T>;

  auto v = ntoh(n);

#ifdef __cpp_lib_bit_cast
  // Thanks to constexpr bit_cast we can evaluate this expression during
  // compilation
  using iterator_value_type = std::remove_cvref_t<
    typename std::iterator_traits<iter_t>::value_type>;
  struct dest {
    alignas(value_type) iterator_value_type value[sizeof(value_type)];
  };
  auto tmp = std::bit_cast<dest>(v);

  std::copy_n(std::begin(tmp.value), sizeof(value_type), dest);
#else
  std::memcpy(&*dest, &v, sizeof(v));
#endif
}

template <std::integral T>
[[nodiscard]] constexpr T 
hton(T n) noexcept {
  return ntoh(n);
}

template <std::integral T, std::contiguous_iterator iter_t>
[[nodiscard]] constexpr T
hton(iter_t src) noexcept {
  return ntoh<T, iter_t>(src);
}

template <std::integral T, endian_impl::contiguous_output_byte_iterator iter_t>
constexpr void
hton(T n, iter_t dest) noexcept {
  return ntoh(n, dest);
}

} // namespace plain

#endif // PLAIN_BASIC_ENDIAN_H_
