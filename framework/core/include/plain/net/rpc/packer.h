/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id packer.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/09/05 16:12
 * @uses The net rpc message packer implemention.
 */

#ifndef PLAIN_NET_RPC_PACKER_H_
#define PLAIN_NET_RPC_PACKER_H_

#include "plain/net/rpc/config.h"
#include <bitset>
#include <cmath>
#include <utility>
#include <chrono>
#include "plain/net/rpc/traits.h"

namespace plain::net {
namespace rpc {

class PLAIN_API Packer {

 public:
  Packer();
  ~Packer();

 public:
  template <typename ...Types>
  void operator()(const Types &...args) noexcept {
    (pack(std::forward<const Types &>(args)), ...);
  }

 public:
  template <typename ...Types>
  void process(const Types &...args) noexcept {
    (pack(std::forward<const Types &>(args)), ...);
  }

  const std::vector<uint8_t> &vector() const noexcept {
    return serialized_object_;
  }

  void clear() noexcept {
    serialized_object_.clear();
  }

 private:
  template <typename T>
  void pack(const T &value) noexcept {
    if constexpr(is_map<T>::value) {
      pack_map(value);
    } else if constexpr (is_container<T>::value || is_stdarray<T>::value) {
      pack_array(value);
    } else if constexpr (is_tuple<T>::value) {
      pack_tuple(value);
    } else {
      auto recursive_packer = Packer{};
      const_cast<T &>(value).pack(recursive_packer);
      pack(recursive_packer.vector());
    }
  }

  template <typename T>
  void pack_array(const T &array) noexcept {
    if (array.size() < 16) {
      auto size_mask = uint8_t(0b10010000);
      serialized_object_.emplace_back(uint8_t(array.size() | size_mask));
    } else if (array.size() < std::numeric_limits<uint16_t>::max()) {
      serialized_object_.emplace_back(std::to_underlying(Type::Array16));
      for (auto i = sizeof(uint16_t); i > 0; --i) {
        serialized_object_.emplace_back(
          uint8_t(array.size() >> (8U * (i - 1)) & 0xff));
      }
    } else if (array.size() < std::numeric_limits<uint32_t>::max()) {
      serialized_object_.emplace_back(std::to_underlying(Type::Array32));
      for (auto i = sizeof(uint32_t); i > 0; --i) {
        serialized_object_.emplace_back(
          uint8_t(array.size() >> (8U * (i - 1)) & 0xff));
      }
    } else {
      return;
    }
    for (const auto &elem : array) {
      pack(elem);
    }
  }

  template <typename T>
  void pack_map(const T &map) noexcept {
    if (map.size() < 16) {
      auto size_mask = uint8_t(0b10000000);
      serialized_object_.emplace_back(uint8_t(map.size() | size_mask));
    } else if (map.size() < std::numeric_limits<uint16_t>::max()) {
      serialized_object_.emplace_back(std::to_underlying(Type::Map16));
      for (auto i = sizeof(uint16_t); i > 0; --i) {
        serialized_object_.emplace_back(
          uint8_t(map.size() >> (8U * (i - 1)) & 0xff));
      }
    } else if (map.size() < std::numeric_limits<uint32_t>::max()) {
      serialized_object_.emplace_back(std::to_underlying(Type::Map32));
      for (auto i = sizeof(uint32_t); i > 0; --i) {
        serialized_object_.emplace_back(
          uint8_t(map.size() >> (8U * (i - 1)) & 0xff));
      }
    } else {
      return;
    }
    for (const auto &elem : map) {
      pack(std::get<0>(elem));
      pack(std::get<1>(elem));
    }
  }

  template <typename T>
  void pack(const std::chrono::time_point<T> &value) {
    pack(value.time_since_epoch().count());
  }

  template <typename T>
  void pack_tuple(const T &value) noexcept {
    std::apply([this](auto &&...args){
      (pack(args), ...);
    }, value);
  }

 public:

  std::bitset<64> twos_complement(int64_t value) noexcept {
    if (value < 0) {
      auto abs_v = std::abs(value);
      return ~abs_v + 1;
    } else {
      return {static_cast<uint64_t>(value)};
    }
  }

  std::bitset<32> twos_complement(int32_t value) noexcept {
    if (value < 0) {
      auto abs_v = std::abs(value);
      return ~abs_v + 1;
    } else {
      return {static_cast<uint32_t>(value)};
    }
  }

  std::bitset<16> twos_complement(int16_t value) noexcept {
    if (value < 0) {
      auto abs_v = std::abs(value);
      return ~abs_v + 1;
    } else {
      return {static_cast<uint16_t>(value)};
    }
  }

  std::bitset<8> twos_complement(int8_t value) noexcept {
    if (value < 0) {
      auto abs_v = std::abs(value);
      return ~abs_v + 1;
    } else {
      return {static_cast<uint8_t>(value)};
    }
  }

 public:

  void pack(int8_t value) noexcept;
  void pack(int16_t value) noexcept;
  void pack(int32_t value) noexcept;
  void pack(int64_t value) noexcept;
  void pack(uint8_t value) noexcept;
  void pack(uint16_t value) noexcept;
  void pack(uint32_t value) noexcept;
  void pack(uint64_t value) noexcept;

 public:
  void pack(std::nullptr_t) noexcept;
  void pack(bool value) noexcept;

 public:
  void pack(float value) noexcept;
  void pack(double value) noexcept;
  void pack(const std::string &value) noexcept;
  void pack(std::string_view value) noexcept {
    pack(std::string{value.data(), value.size()});
  }
  void pack(const char *str) noexcept {
    pack(std::string{str});
  }
  void pack(const std::vector<uint8_t> &value) noexcept;

 private:
  std::vector<uint8_t> serialized_object_;

};

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_PACKER_H_
