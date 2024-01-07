/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id variable.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/10/16 16:37
 * @uses Base module the variable type(like script variables).
 *       Base on string, also can do string convert to diffrent type.
 *       Abstract data type(ADT).
 *       Refer: php/lua or more script codes.
 *
 *      FIXME: optimize the operator to safe.
*/
#ifndef PLAIN_BASIC_TYPE_VARIABLE_H_
#define PLAIN_BASIC_TYPE_VARIABLE_H_

#include "plain/basic/type/config.h"
#include <utility>
#include <variant>
#include "plain/basic/concepts.h"

namespace plain {

struct PLAIN_API variable_struct {
  using value_type = std::variant<
    bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
    uint64_t, float, double, std::string>;
  mutable std::mutex mutex;
  value_type value;
  variable_struct() {}

  variable_struct(const variable_t &object) {
    value = object.value;
  } 
  variable_struct(variable_t &&object) {
    std::swap(value, object.value);
  }
  template <typename T>
  requires(!std::is_same_v<T, value_type> and !std::is_enum_v<T>)
  variable_struct(T _value) {
    value = _value;
  }
  variable_struct(enums auto _value) {
    value = std::to_underlying(_value);
  }
  
  template <typename T>
  T get() const {
    std::unique_lock<std::mutex> auto_lock(mutex);
    return std::get<T>(value);
  } 
  template <typename T>
  T _get() const {
    return std::get<T>(value);
  } //Not safe in multi threads.

  const char *c_str() {
    return std::get<std::string>(value).c_str();
  }

  int64_t to_int64() const noexcept {
    int64_t r{0};
    std::visit([&r](auto &&val) {
      using type = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<type, std::string>) {
        char *endpointer = nullptr;
        r = strtoint64(val.c_str(), &endpointer, 10);
      } else {
        r = static_cast<int64_t>(val);
      }
    }, value);
    return r;
  }

  double to_double() const noexcept {
    double r{0.0};
    std::visit([&r](auto &&val) {
      using type = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<type, std::string>) {
        r = static_cast<double>(atof(val.c_str()));
      } else {
        r = static_cast<double>(val);
      }
    }, value);
    return r;
  }

  variable_t &operator=(const variable_t &object) = default;
  variable_t &operator=(variable_t &&object) {
    std::swap(object.value, value);
    return *this;
  }

  variable_t &operator+=(const variable_t &object) noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = object.to_int64() + to_double();
    return *this;
  }
  template <typename T>
  variable_t &operator+=(T _value) {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = _value + to_double();
    return *this;
  }

  variable_t &operator-=(const variable_t &object) noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_double() - object.to_int64();
    return *this;
  }
  template <typename T>
  variable_t &operator -= (T _value) {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_double() - _value;
    return *this;
  }

  variable_t &operator*=(const variable_t &object) noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_double() * object.to_int64();
    return *this;
  }
  
  template <typename T>
  variable_t &operator *= (T _value) {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_double() * _value;
    return *this;
  }

  variable_t &operator/=(const variable_t &object) {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_double() / object.to_int64();
    return *this;
  }
  template <typename T>
  variable_t &operator /= (T _value) {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_double() / _value;
    return *this;
  }

  variable_t &operator++() noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_int64() + 1;
    return *this;
  }
  variable_t &operator--() noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_int64() - 1;
    return *this;
  }
  variable_t &operator++(int32_t) noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_int64() + 1;
    return *this;
  }
  variable_t &operator--(int32_t) noexcept {
    std::unique_lock<std::mutex> auto_lock(mutex);
    value = to_int64() + 1;
    return *this;
  }
  
  friend bool operator==(
      const variable_t &lhs, const variable_t &rhs) noexcept {
    return lhs.value == rhs.value;
  }
  
  friend bool operator!=(const variable_t &lhs, const variable_t &rhs) noexcept {
    return lhs.value != rhs.value;
  }

  friend bool operator<(const variable_t &lhs, const variable_t &rhs) noexcept {
    return lhs.value < rhs.value;
  }

  friend bool operator>(const variable_t &lhs, const variable_t &rhs) noexcept {
    return lhs.value < rhs.value;
  }

  friend auto operator<=>(
      const variable_t &lhs, const variable_t &rhs) noexcept {
    return lhs.value <=> rhs.value;
  }

  operator const char*() {
    std::unique_lock<std::mutex> auto_lock(mutex);
    return std::get<std::string>(value).c_str();
  }
  template <typename T>
  operator T() {
    std::unique_lock<std::mutex> auto_lock(mutex);
    return std::get<T>(value);
  }

}; //PLAIN变量，类似脚本变量

// FIXME: This not implicit any type to variable_t when test(gcc 13.1).
std::ostream &operator<<(std::ostream &os, const variable_t &object) noexcept;

std::ostream &operator<<(std::ostream &os, const bytes_t &bytes) noexcept;

} //namespace plain

#endif //PLAIN_BASIC_TYPE_VARIABLE_H_
