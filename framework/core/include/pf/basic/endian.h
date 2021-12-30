/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id endian.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2021 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2021/12/03 14:22
 * @uses The basic endian.
 *       refer: https://github.com/mandreyel/endian
 */

#ifndef PF_BASIC_ENDIAN_H_
#define PF_BASIC_ENDIAN_H_

#include "pf/basic/config.h"

#include <cstddef> // for __GLIBC__

// Ported from boost/predef/other/endian.h.

//  Copyright (C) 2012 David Stone
//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt
#if defined(__GLIBC__) \
 || defined(__GLIBCXX__) \
 || defined(__GNU_LIBRARY__) \
 || defined(__ANDROID__)
# include <endian.h>
#else
# if defined(macintosh) \
  || defined(Macintosh) \
  || (defined(__APPLE__) && defined(__MACH__))
#  include <machine/endian.h>
# elif defined(BSD) || defined(_SYSTYPE_BSD)
#  if defined(__OpenBSD__)
#   include <machine/endian.h>
#  else
#   include <sys/endian.h>
#  endif
/* Try to derive from arch/compiler-specific macros */
# elif defined(_X86_) || defined(__x86_64__) || defined(__i386__) || \
   defined(__i486__) || defined(__i586__) || defined(__i686__) || \
   defined(__MIPSEL) || defined(_MIPSEL) || defined(MIPSEL) || \
   defined(__ARMEL__) || \
   defined(__MSP430__) || \
   (defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__ == 1) || \
   (defined(_LITTLE_ENDIAN) && _LITTLE_ENDIAN == 1) || \
   defined(_M_ARM) || defined(_M_ARM64) || \
   defined(_M_IX86) || defined(_M_AMD64) /* MSVC */ //little endian
#   ifndef _BYTE_ORDER
#     define _BYTE_ORDER 0
#     define _LITTLE_ENDIAN 0
#     define _BIG_ENDIAN 1
#   endif
# endif
#endif

#if defined(__BYTE_ORDER)
# if defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
#  define PF_BIG_ENDIAN
# elif defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define PF_LITTLE_ENDIAN
# else
#  define PF_UNKNOWN_ENDIANNESS
# endif
#elif defined(_BYTE_ORDER)
# if defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN)
#  define PF_BIG_ENDIAN
# elif defined(_LITTLE_ENDIAN) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#  define PF_LITTLE_ENDIAN
# else
#  define PF_UNKNOWN_ENDIANNESS
# endif
#else
# define PF_UNKNOWN_ENDIANNESS
#endif

#ifndef __has_builtin
  #define __has_builtin(x) 0  // Compatibility with non-clang compilers.
#endif

#if defined(_MSC_VER)
// Microsoft documents these as being compatible since Windows 95 and
// specifically lists runtime library support since Visual Studio 2003 (aka
// 7.1).  Clang/c2 uses the Microsoft rather than GCC intrinsics, so we check
// for defined(_MSC_VER) before defined(__clang__).
# include <cstdlib>
# define PF_BYTE_SWAP_16(x) _byteswap_ushort(x)
# define PF_BYTE_SWAP_32(x) _byteswap_ulong(x)
# define PF_BYTE_SWAP_64(x) _byteswap_uint64(x)

//  GCC and Clang recent versions provide intrinsic byte swaps via builtins.
#elif (defined(__clang__) \
  && __has_builtin(__builtin_bswap32) \
  && __has_builtin(__builtin_bswap64)) \
  || (defined(__GNUC__ ) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
// Prior to 4.8, gcc did not provide __builtin_bswap16 on some platforms so we 
// emulate it see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52624.
// Clang has a similar problem, but their feature test macros make it easier 
// to detect.
# if (defined(__clang__) && __has_builtin(__builtin_bswap16)) \
  || (defined(__GNUC__) &&(__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#   define PF_BYTE_SWAP_16(x) __builtin_bswap16(x)
# else
#   define PF_BYTE_SWAP_16(x) __builtin_bswap32((x) << 16)
# endif
# define PF_BYTE_SWAP_32(x) __builtin_bswap32(x)
# define PF_BYTE_SWAP_64(x) __builtin_bswap64(x)

//  Linux systems provide the byteswap.h header.
#elif defined(__linux__)
# include <byteswap.h>
# define PF_BYTE_SWAP_16(x) bswap_16(x)
# define PF_BYTE_SWAP_32(x) bswap_32(x)
# define PF_BYTE_SWAP_64(x) bswap_64(x)

// We need to provide our own implementation.
#else
namespace pf_basic {

namespace endian {

namespace detail {

constexpr uint16_t swap_u16(uint16_t n) noexcept {
  return (n << 8) | (n >> 8);
}

constexpr uint32_t swap_u32(uint32_t n) noexcept {
  n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0xFF00FF);
  return (n << 16) | (n >> 16);
}

constexpr uint64_t swap_u64(uint64_t n) noexcept {
  n = ((n << 8)  & 0xFF00FF00FF00FF00ull) | ((n >> 8)  & 0x00FF00FF00FF00FFull);
  n = ((n << 16) & 0xFFFF0000FFFF0000ull) | ((n >> 16) & 0x0000FFFF0000FFFFull);
  return (n << 32) | (n >> 32);
}

} // detail

} // endian

} // namespace pf_basic

# define PF_BYTE_SWAP_16(x) endian::detail::swap_u16(static_cast<uint16_t>(x))
# define PF_BYTE_SWAP_32(x) endian::detail::swap_u32(static_cast<uint32_t>(x))
# define PF_BYTE_SWAP_64(x) endian::detail::swap_u64(static_cast<uint64_t>(x))
#endif

// -- type traits

#include <type_traits>
#include <cstdint>

namespace pf_basic {

namespace endian {

namespace detail {

template<typename T>
struct is_endian_reversible {
  // TODO: is is_pod right?
  static constexpr bool value = std::is_integral<T>::value || 
  std::is_pod<T>::value;
};

template<typename... Ts>
struct make_void { using type = void; };

template<typename... Ts>
using void_t = typename make_void<Ts...>::type;

template<typename T, typename>
struct is_input_iterator : std::false_type {};

template<typename T>
struct is_input_iterator<T, void_t<
    decltype(*std::declval<T&>()),
    decltype(++std::declval<T&>()),
    decltype(std::declval<T&>()++)>>
  : std::true_type {};

template <size_t N>
struct integral_type_for {
  static_assert(N <= 8, "N may be at most 8 bytes large");
  using type = typename std::conditional<
    (N > sizeof(uint8_t)),
    typename std::conditional<
      (N > sizeof(uint16_t)),
      typename std::conditional<
        (N > sizeof(uint32_t)),
        uint64_t,
        uint32_t
      >::type,
      uint16_t
    >::type,
    uint8_t
  >::type;
};

} // detail

} // endian

// -- API

namespace endian {

enum order {
  little,
  big,
  network = big,
#if defined(PF_BIG_ENDIAN)
  host = big,
#elif defined(PF_LITTLE_ENDIAN)
  host = little,
#endif
};

/**
 * Parses `sizeof(T)` bytes from the memory pointed to by `it`, 
 * and reconstructs from it an integer of type `T`, 
 * converting from the specified `Order` to host byte order.
 *
 * The value type of the iterator must represent a byte, that is:
 * `sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(char)`.
 *
 * It's undefined behaviour if `it` points to a buffer smaller than `sizeof(T)` 
 * bytes.
 *
 * The byte sequence must have at least `sizeof(T)` bytes.
 * `Order` must be either `endian::order::big`, `endian::order::little`,
 * `endian::order::network`, or `endian::order::host`.
 *
 * This is best used when data received during IO is read into a buffer and 
 * numbers need to be read from it. E.g.:
 * ```
 * std::array<char, 1024> buffer;
 * // Receive into `buffer`.
 * // ...
 * // Assume that the first four bytes in `buffer` constitute a 32-bit big 
 * endian integer.
 * int32_t n = endian::read<endian::order::big, int32_t>(buffer.data());
 * ```
 */
template<order Order, class T, class InputIt>
PF_CONSTEXPR T read(InputIt it) noexcept;

template<class T, class InputIt>
PF_CONSTEXPR T read_le(InputIt it) noexcept {
  return read<order::little, T>(it);
}

template<class T, class InputIt>
PF_CONSTEXPR T read_be(InputIt it) noexcept {
  return read<order::big, T>(it);
}

/**
 * Parses `N` bytes from the memory pointed to by `it`, and reconstructs from it
 * an unsigned integer of type `T` that is at least as large to fit `N` bytes
 * (but at most 8 bytes), converting from the specified `Order` to host byte
 * order.
 *
 * The value type of the iterator must represent a byte, that is:
 * `sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(char)`.
 *
 * It's undefined behaviour if `it` points to a buffer smaller than `sizeof(T)` 
 * bytes.
 *
 * The byte sequence must have at least `sizeof(T)` bytes.
 * `Order` must be either `endian::order::big`, `endian::order::little`,
 * `endian::order::network`, or `endian::order::host`.
 *
 * This is best used when data received during IO is read into a buffer and 
 * numbers need to be read from it. E.g.:
 * ```
 * std::array<char, 1024> buffer;
 * // Receive into `buffer`.
 * // ...
 * // Assume that the first three bytes in `buffer` constitute a 24-bit big 
 * // endian nteger.
 * int32_t n = endian::read<endian::order::big, 3>(buffer.data());
 * ```
 */
template<order Order, size_t N, class InputIt,
  class T = typename detail::integral_type_for<N>::type>
PF_CONSTEXPR T read(InputIt it) noexcept;

template<size_t N, class InputIt,
  class T = typename detail::integral_type_for<N>::type>
PF_CONSTEXPR T read_le(InputIt it) noexcept {
  return read<order::little, N>(it);
}

template<size_t N, class InputIt,
  class T = typename detail::integral_type_for<N>::type>
PF_CONSTEXPR T read_be(InputIt it) noexcept {
  return read<order::big, N>(it);
}

// DEPRECATED: use `read` instead.
template<order Order, class T, class InputIt>
PF_CONSTEXPR T parse(InputIt it) noexcept { return read<Order, T>(it); }

/**
 * Writes each byte of `h` to the memory pointed to by `it`, 
 * such that it converts the
 * byte order of `h` from host byte order to the specified `Order`.
 *
 * The value type of the iterator must represent a byte, that is:
 * `sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(char)`.
 *
 * It's undefined behaviour if `it` points to a buffer smaller than `sizeof(T)`
 * bytes.
 *
 * The byte sequence must have at least `sizeof(T)` bytes.
 * `Order` must be either `endian::order::big`, `endian::order::little`,
 * `endian::order::network`, or `endian::order::host`.
 *
 * This is best used when data transferred during IO is written to a buffer 
 * first, and
 * among the data to be written are integers. E.g.:
 * ```
 * std::array<char, 1024> buffer;
 * const int32_t number = 42;
 * // Write `number` as a big endian number to `buffer`.
 * endian::write<endian::order::big>(number, &buffer[0]);
 * // Write `number` as a little endian number to `buffer`.
 * endian::write<endian::order::little>(number, &buffer[4]);
 * ```
 */
template<order Order, class T, class OutputIt>
PF_CONSTEXPR void write(const T &h, OutputIt it) noexcept;

template<class T, class OutputIt>
PF_CONSTEXPR void write_le(const T &h, OutputIt it) noexcept {
  write<order::little>(h, it);
}

template<class T, class OutputIt>
PF_CONSTEXPR void write_be(const T &h, OutputIt it) noexcept {
  write<order::big>(h, it);
}

/**
 * Writes the first `N` byte of `h` to the memory pointed to by `it`,
 * such that it converts the
 * byte order of `h` from host byte order to the specified `Order`. Thus `N`
 * must be at most `sizeof(h)`.
 *
 * The value type of the iterator must represent a byte, that is:
 * `sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(char)`.
 *
 * It's undefined behaviour if `it` points to a buffer smaller than `N` bytes.
 *
 * The byte sequence must have at least `N` bytes.
 * `Order` must be either `endian::order::big`, `endian::order::little`,
 * `endian::order::network`, or `endian::order::host`.
 *
 * This is best used when data transferred during IO is written to a buffer
 * first, and
 * among the data to be written are integers. E.g.:
 * ```
 * std::array<char, 1024> buffer;
 * const int32_t number = 42;
 * // Write a three byte `number` as a big endian number to `buffer`.
 * endian::write<endian::order::big, 3>(number, &buffer[0]);
 * // Write a three byte `number` as a little endian number to `buffer`.
 * endian::write<endian::order::little, 3>(number, &buffer[4]);
 * ```
 */
template<order Order, size_t N, class T, class OutputIt>
PF_CONSTEXPR void write(const T &h, OutputIt it) noexcept;

template<size_t N, class T, class OutputIt>
PF_CONSTEXPR void write_le(const T &h, OutputIt it) noexcept {
  write<order::little, N>(h, it);
}

template<size_t N, class T, class OutputIt>
PF_CONSTEXPR void write_be(const T &h, OutputIt it) noexcept {
  write<order::big, N>(h, it);
}

/**
 * Reverses endianness, i.e. the byte order in `t`. E.g. given the 16-bit number
 * '0x1234', this function returns '0x4321'.
 */
template<class T>
PF_CONSTEXPR T reverse(const T &t);

#ifndef PF_UNKNOWN_ENDIANNESS
/// These are only available if your platform has a defined endianness.
/**
 * Conditionally converts to the specified endianness if and only if the host's
 * byte order differs from `Order`.
 */
template<order Order, class T>
PF_CONSTEXPR T conditional_convert(const T &t) noexcept;

/**
 * Conditionally converts to network byte order if and only if the host's byte 
 * order is different from the network byte order.
 *
 * Functionally equivalent to the POSIX ntoh* functions or to this code:
 * ```
 * if(endian::order::host != endian::order::network)
 *   t = endian::reverse(t);
 * ```
 * Or:
 * ```
 * t = endian::conditional_convert<endian::order::network>(t);
 * ```
 */
template<class T>
PF_CONSTEXPR T host_to_network(const T &t);

/**
 * Conditionally converts to host byte order if and only if the host's byte 
 * order is different from the network byte order.
 *
 * Functionally equivalent to the POSIX ntoh* functions or to this code:
 * ```
 * if(endian::order::host != endian::order::network)
 *   t = endian::reverse(t);
 * ```
 * Or:
 * ```
 * t = endian::conditional_convert<endian::order::network>(t);
 * ```
 */
template<class T>
PF_CONSTEXPR T network_to_host(const T &t);
#endif // PF_UNKNOWN_ENDIANNESS

} // endian

// -- implementation

namespace endian {

namespace detail {

/** Reads an integer of type `T` from buffer pointed to by `it` and converts it
 * from BIG endian order.
 */
template<order Order, class T, class InputIt, size_t MaxNBytes = sizeof(T)>
PF_CONSTEXPR typename std::enable_if<Order == order::big, T>::type
read(InputIt it) noexcept {
  static_assert(
      sizeof(T) >= MaxNBytes, "Can only read at most sizeof(T) bytes");
  T h = 0;
  for(int i = 0; i < int(MaxNBytes); ++i) {
    h <<= 8;
    h |= static_cast<uint8_t>(*it++);
  }
  return h;
}

/**
 * Reads an integer of type `T` from buffer pointed to by `it` and converts it
 * from LITTLE endian order.
 */
template<order Order, class T, class InputIt, size_t MaxNBytes = sizeof(T)>
PF_CONSTEXPR typename std::enable_if<Order == order::little, T>::type
read(InputIt it) noexcept {
  static_assert(
      sizeof(T) >= MaxNBytes, "Can only read at most sizeof(T) bytes");
  T h = 0;
  for(int i = 0; i < int(MaxNBytes); ++i)
  {
    h |= static_cast<uint8_t>(*it++) << i * 8;
  }
  return h;
}

// --

/** Converts `h` to BIG endian order, 
 * writing it to buffer pointed to by `it`. */
template<order Order, class T, class OutputIt, size_t MaxNBytes = sizeof(T)>
PF_CONSTEXPR typename std::enable_if<Order == order::big, void>::type
write(const T &h, OutputIt it) noexcept {
  static_assert(
      sizeof(T) >= MaxNBytes, "Can only write at most sizeof(T) bytes");
  for(int shift = 8 * (int(MaxNBytes) - 1); shift >= 0; shift -= 8) {
    *it++ = static_cast<uint8_t>((h >> shift) & 0xff);
  }
}

/** Converts `h` to LITTLE endian order, 
 * writing it to buffer pointed to by `it`. */
template<order Order, class T, class OutputIt, size_t MaxNBytes = sizeof(T)>
PF_CONSTEXPR typename std::enable_if<Order == order::little, void>::type
write(const T &h, OutputIt it) noexcept {
  static_assert(
      sizeof(T) >= MaxNBytes, "Can only write at most sizeof(T) bytes");
  for(int i = 0; i < int(MaxNBytes); ++i)
  {
    *it++ = static_cast<uint8_t>((h >> i * 8) & 0xff);
  }
}

// --

template<size_t Size>
struct byte_swapper {};

template<>
struct byte_swapper<2> {
  template<class T>
  T operator()(const T &t) { return PF_BYTE_SWAP_16(t); }
};

template<>
struct byte_swapper<4> {
  template<class T>
  T operator()(const T &t) { return static_cast<T>(PF_BYTE_SWAP_32(static_cast<uint32_t>(t))); }
};

template<>
struct byte_swapper<8> {
  template<class T>
  T operator()(const T &t) { return static_cast<T>(PF_BYTE_SWAP_64(static_cast<uint64_t>(t))); }
};

// --

#ifndef PF_UNKNOWN_ENDIANNESS
template<order Order>
struct conditional_reverser {
  template<class T>
  PF_CONSTEXPR T operator()(const T &t) { return reverse(t); }
};

template<>
struct conditional_reverser<order::host> {
  template<class T>
  PF_CONSTEXPR T operator()(const T &t) { return t; }
};
#endif // PF_UNKNOWN_ENDIANNESS

} // detail

template<order Order, class T, class InputIt>
PF_CONSTEXPR T read(InputIt it) noexcept {
  static_assert(detail::is_endian_reversible<T>::value,
    "T must be an integral or POD type");
  //static_assert(detail::is_input_iterator<InputIt>::value,
    //"Iterator type requirements not met");
  return detail::read<Order, T>(it);
}

template<order Order, size_t N, class InputIt, class T>
PF_CONSTEXPR T read(InputIt it) noexcept {
  static_assert(detail::is_endian_reversible<T>::value,
    "T must be an integral or POD type");
  // Read at most `N` bytes from `it`.
  return detail::read<Order, T, InputIt, N>(it);
}

template<order Order, class T, class OutputIt>
PF_CONSTEXPR void write(const T &h, OutputIt it) noexcept {
  static_assert(detail::is_endian_reversible<T>::value,
    "T must be an integral or POD type");
  //static_assert(detail::is_input_iterator<OutputIt>::value,
    //"Iterator type requirements not met");
  detail::write<Order, T>(h, it);
}

template<order Order, size_t N, class T, class OutputIt>
PF_CONSTEXPR void write(const T &h, OutputIt it) noexcept {
  static_assert(detail::is_endian_reversible<T>::value,
    "T must be an integral or POD type");
  detail::write<Order, T, OutputIt, N>(h, it);
}

template<class T>
PF_CONSTEXPR T reverse(const T &t) {
  return detail::byte_swapper<sizeof t>()(t);
}

#ifndef PF_UNKNOWN_ENDIANNESS
template<order Order, class T>
PF_CONSTEXPR T conditional_convert(const T &t) noexcept {
  return detail::conditional_reverser<Order>()(t);
}

template<class T>
PF_CONSTEXPR T host_to_network(const T &t) {
  return conditional_convert<order::network>(t);
}

template<class T>
PF_CONSTEXPR T network_to_host(const T &t) {
  // hton and ntoh are essentially the same, 
  // as they both do a byte swap if and only
  // if the host's and network's byte orders differ.
  return host_to_network(t);
}
#endif // PF_UNKNOWN_ENDIANNESS

} // endian

} // namespace pf_basic

// useful macros.
#ifndef PF_NTOH
#define PF_NTOH pf_basic::endian::network_to_host
#endif
#ifndef PF_HTON
#define PF_HTON pf_basic::endian::host_to_network
#endif

#endif // PF_BASIC_ENDIAN_H_
