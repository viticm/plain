/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id byte.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/21 10:24
 * @uses The byte useful implemention.
 */

#ifndef PLAIN_BASIC_TYPE_BYTE_H_
#define PLAIN_BASIC_TYPE_BYTE_H_

#include "plain/basic/type/config.h"
#include <span>

namespace plain {

using byte_span_t = std::span<std::byte>;
using const_byte_span_t = std::span<const std::byte>;

template <class T>
constexpr byte_span_t as_bytes(T &container) noexcept {
  return {reinterpret_cast<std::byte *>(container.data()), container.size()};
}

template <class T>
constexpr byte_span_t as_bytes(T *ptr, std::size_t size) noexcept {
  return {reinterpret_cast<std::byte *>(ptr), size};
}

template <class T>
constexpr const_byte_span_t as_const_bytes(const T &container) noexcept {
  return {reinterpret_cast<const std::byte *>(container.data()),
          container.size()};
}

template <class T>
constexpr const_byte_span_t as_const_bytes(T *ptr, std::size_t size) noexcept {
  return {reinterpret_cast<const std::byte *>(ptr), size};
}

} // namespace plain

#endif // PLAIN_BASIC_TYPE_BYTE_H_
