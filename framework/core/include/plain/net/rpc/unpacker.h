/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id unpacker.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/09/05 17:12
 * @uses The net rpc unpacker implemention.
 */

#ifndef PLAIN_NET_RPC_UNPACKER_H_
#define PLAIN_NET_RPC_UNPACKER_H_

#include "plain/net/rpc/config.h"
#include <chrono>
#include "plain/basic/error.h"
#include "plain/net/rpc/traits.h"

namespace plain::net {
namespace rpc {

class PLAIN_API Unpacker {

 public:
  Unpacker();
  Unpacker(const uint8_t *data, size_t size);
  ~Unpacker();

 public:
  Error error_;

 public:
  uint8_t data() noexcept {
    if (data_ < data_end_)
      return *data_;
    error_ = ErrorCode::OutOfRange;
    return 0;
  }

  void set_data(const uint8_t *data, const size_t size) {
    data_ = data;
    data_end_ = data + size;
  }

  void increment(int64_t bytes = 1) {
    if (data_end_ - data_ >= bytes) {
      data_ += bytes;
    } else {
      error_ = ErrorCode::OutOfRange;
    }
  }

 public:
  template <typename ...Types>
  void operator()(Types &...args) noexcept {
    (unpack(std::forward<Types &>(args)), ...);
  }

 public:
  template <typename ...Types>
  void process(Types &...args) noexcept {
    (unpack(std::forward<Types &>(args)), ...);
  }

 public:

  template <typename T>
  void unpack(T &value) noexcept {
    if constexpr(is_map<T>::value) {
      unpack_map(value);
    } else if constexpr (is_container<T>::value) {
      unpack_array(value);
    } else if constexpr (is_stdarray<T>::value) {
      unpack_stdarray(value);
    } else {
      auto recursive_data = std::vector<uint8_t>{};
      unpack(recursive_data);

      auto recursive_unpacker = 
        Unpacker{recursive_data.data(), recursive_data.size()};

      value.pack(recursive_unpacker);
      error_ = recursive_unpacker.error_;
    }

  }

  template <typename Clock, typename Duration>
  void unpack(std::chrono::time_point<Clock, Duration> &value) noexcept {
    using RepType = typename std::chrono::time_point<Clock, Duration>::rep;
    using DurationType = Duration;
    using TimepointType = typename std::chrono::time_point<Clock, Duration>;
    auto placeholder = RepType{};
    unpack(placeholder);
    value = TimepointType(DurationType(placeholder));
  }

  template <typename T>
  void unpack_array(T &array) noexcept {
    using ValueType = typename T::value_type;
    if (data() == std::to_underlying(Type::Array32)) {
      increment();
      std::size_t array_size = 0;
      for (auto i = sizeof(uint32_t); i > 0; --i) {
        array_size += uint32_t(data()) << 8 * (i - 1);
        increment();
      }
      std::vector<uint32_t> x{};
      for (auto i = 0U; i < array_size; ++i) {
        ValueType val{};
        unpack(val);
        array.emplace_back(val);
      }
    } else if (data() == std::to_underlying(Type::Array16)) {
      increment();
      std::size_t array_size = 0;
      for (auto i = sizeof(uint16_t); i > 0; --i) {
        array_size += uint16_t(data()) << 8 * (i - 1);
        increment();
      }
      for (auto i = 0U; i < array_size; ++i) {
        ValueType val{};
        unpack(val);
        array.emplace_back(val);
      }
    } else {
      std::size_t array_size = data() & 0b00001111;
      increment();
      for (auto i = 0U; i < array_size; ++i) {
        ValueType val{};
        unpack(val);
        array.emplace_back(val);
      }
    }
  }

  template <typename T>
  void unpack_stdarray(T &array) noexcept {
    using ValueType = typename T::value_type;
    auto vec = std::vector<ValueType>{};
    unpack_array(vec);
    std::copy(vec.begin(), vec.end(), array.begin());
  }

  template <typename T>
  void unpack_map(T &map) noexcept {
    using KeyType = typename T::key_type;
    using MappedType = typename T::mapped_type;
    if (data() == std::to_underlying(Type::Map32)) {
      increment();
      std::size_t map_size = 0;
      for (auto i = sizeof(uint32_t); i > 0; --i) {
        map_size += uint32_t(data()) << 8 * (i - 1);
        increment();
      }
      std::vector<uint32_t> x{};
      for (auto i = 0U; i < map_size; ++i) {
        KeyType key{};
        MappedType value{};
        unpack(key);
        unpack(value);
        map.insert_or_assign(key, value);
      }
    } else if (data() == std::to_underlying(Type::Map16)) {
      increment();
      std::size_t map_size = 0;
      for (auto i = sizeof(uint16_t); i > 0; --i) {
        map_size += uint16_t(data()) << 8 * (i - 1);
        increment();
      }
      for (auto i = 0U; i < map_size; ++i) {
        KeyType key{};
        MappedType value{};
        unpack(key);
        unpack(value);
        map.insert_or_assign(key, value);
      }
    } else {
      std::size_t map_size = data() & 0b00001111;
      increment();
      for (auto i = 0U; i < map_size; ++i) {
        KeyType key{};
        MappedType value{};
        unpack(key);
        unpack(value);
        map.insert_or_assign(key, value);
      }
    }
  }

 public:
  void unpack(int8_t &value) noexcept;
  void unpack(int16_t &value) noexcept;
  void unpack(int32_t &value) noexcept;
  void unpack(int64_t &value) noexcept;
  void unpack(uint8_t &value) noexcept;
  void unpack(uint16_t &value) noexcept;
  void unpack(uint32_t &value) noexcept;
  void unpack(uint64_t &value) noexcept;
  
 public:
  void unpack(bool &value) noexcept;
  void unpack(float &value) noexcept;
  void unpack(double &value) noexcept;
  void unpack(std::string &value) noexcept;
  void unpack(std::nullptr_t &) noexcept;
  void unpack(std::vector<uint8_t> &value) noexcept;

 private:
  const uint8_t *data_{nullptr};
  const uint8_t *data_end_{nullptr};

};

} // namespace rpc
} // namespace plain::net

#endif // PLAIN_NET_RPC_UNPACKER_H_
