/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id variable.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2023/03/31 18:08
 * @uses The variable implement.
*/
#ifndef PLAIN_BASIC_TYPE_VARIABLE_TCC_
#define PLAIN_BASIC_TYPE_VARIABLE_TCC_

#include "plain/basic/type/variable.h"

namespace plain {

template <typename T>
Variable std_convert_type(T) {
  using enum Variable;
  if (is_same(bool, T)) return Bool;
  if (is_same(int32_t, T)) return Int32;
  if (is_same(uint32_t, T)) return Uint32;
  if (is_same(int16_t, T)) return Int16;
  if (is_same(uint16_t, T)) return Uint16;
  if (is_same(int8_t, T)) return Int8;
  if (is_same(uint8_t, T)) return Uint8;
  if (is_same(int64_t, T)) return Int64;
  if (is_same(uint64_t, T)) return Uint64;
  if (is_same(float, T)) return Float;
  if (is_same(double, T)) return Double;
  return String; //Default is string.
}

inline variable_struct::variable_struct(const variable_t &object) {
  data = object.data;
  type = object.type;
}
  
inline variable_struct::variable_struct(const variable_t *object) {
  if (object) {
    data = object->data;
    type = object->type;
  }
}

inline variable_struct::variable_struct(const std::string &value) {
  type = String;
  data = value;
}

inline variable_struct::variable_struct(const char *value) {
  if (value) {
    type = String;
    data = value;
  }
}
  
template <typename T>
inline variable_struct::variable_struct(T value) {
  type = std_convert_type(value);
  data = std::to_string(value);
}
 
template <typename T>
inline T variable_struct::_get() const {
  T result{(T)0};
  if (is_same(float, T)) {
    result = static_cast<T>(atof(data.c_str()));
  } else if (is_same(double, T)) {
    result = static_cast<T>(atof(data.c_str()));
  } else {
    char *endpointer = nullptr;
    int64_t temp = strtoint64(data.c_str(), &endpointer, 10);
    result = static_cast<T>(temp);
  }
  return result;
}

template <>
inline bool variable_struct::_get<bool>() const {
  return data != "0" && data != "";
}

template <>
inline std::string variable_struct::_get<std::string>() const {
  return data;
}
  
template <typename T>
inline T variable_struct::get() const {
  std::unique_lock<std::mutex> auto_lock(mutex);
  return _get<T>();
}

inline const char * variable_struct::c_str() const {
  return data.c_str();
}

inline variable_t &variable_struct::operator = (const variable_t &object) {
  if (this == &object) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = object.type;
  data = object.data;
  return *this;
}

inline variable_t *variable_struct::operator = (const variable_t *object) {
  if (this == object) return this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  if (object) {
    type = object->type;
    data = object->data;
  }
  return this;
}

inline variable_t &variable_struct::operator = (const std::string &value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = String;
  data = value;
  return *this;
}

inline variable_t &variable_struct::operator = (const char *value) {
  if (is_null(value)) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = String;
  data = value;
  return *this;
}
  
inline variable_t &variable_struct::operator = (char *value) {
  if (is_null(value)) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = String;
  data = value;
  return *this;
}

template <typename T>
inline variable_t &variable_struct::operator = (T value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = std_convert_type(value);
  data = std::to_string(value);
  return *this;
}

inline variable_t &variable_struct::operator += (const variable_t &object) {
  switch (type) {
    case Int32:
      *this += object.get<int32_t>();
      break;
    case Uint32:
      *this += object.get<uint32_t>();
      break;
    case Int16:
      *this += object.get<int16_t>();
      break;
    case Uint16:
      *this += object.get<uint16_t>();
      break;
    case Int8:
      *this += object.get<int8_t>();
      break;
    case Uint8:
      *this += object.get<uint8_t>();
      break;
    case Int64:
      *this += object.get<int64_t>();
      break;
    case Uint64:
      *this += object.get<uint64_t>();
      break;
    case Float:
      *this += object.get<float>();
      break;
    case Double:
      *this += object.get<double>();
      break;
    default:
      *this += object.data;
      break;
  }
  return *this;
}

inline variable_t *variable_struct::operator += (const variable_t *object) {
  if (object) *this += *object;
  return this;
}

inline variable_t &variable_struct::operator += (const std::string &value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = String;
  data += value;
  return *this;
}

inline variable_t &variable_struct::operator += (const char *value) {
  if (is_null(value)) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = String;
  data += value;
  return *this;
}

template <typename T>
inline variable_t &variable_struct::operator += (T value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  auto last = _get<T>();
  last += value;
  data = std::to_string(last);
  return *this;
}

inline variable_t &variable_struct::operator -= (const variable_t &object) {
  switch (type) {
    case Int32:
      *this -= object.get<int32_t>();
      break;
    case Uint32:
      *this -= object.get<uint32_t>();
      break;
    case Int16:
      *this -= object.get<int16_t>();
      break;
    case Uint16:
      *this -= object.get<uint16_t>();
      break;
    case Int8:
      *this -= object.get<int8_t>();
      break;
    case Uint8:
      *this -= object.get<uint8_t>();
      break;
    case Int64:
      *this -= object.get<int64_t>();
      break;
    case Uint64:
      *this -= object.get<uint64_t>();
      break;
    case Float:
      *this -= object.get<float>();
      break;
    case Double:
      *this -= object.get<double>();
      break;
    default:
      break;
  }
  return *this;
}

inline variable_t *variable_struct::operator -= (const variable_t *object) {
  if (object) *this = *object;
  return this;
}
  
template <typename T>
inline variable_t &variable_struct::operator -= (T value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  auto last = _get<T>();
  last -= value;
  data = std::to_string(last);
  return *this;
}

inline variable_t &variable_struct::operator *= (const variable_t &object) {
  switch (type) {
    case Int32:
      *this *= object.get<int32_t>();
      break;
    case Uint32:
      *this *= object.get<uint32_t>();
      break;
    case Int16:
      *this *= object.get<int16_t>();
      break;
    case Uint16:
      *this *= object.get<uint16_t>();
      break;
    case Int8:
      *this *= object.get<int8_t>();
      break;
    case Uint8:
      *this *= object.get<uint8_t>();
      break;
    case Int64:
      *this *= object.get<int64_t>();
      break;
    case Uint64:
      *this *= object.get<uint64_t>();
      break;
    case Float:
      *this *= object.get<float>();
      break;
    case Double:
      *this *= object.get<double>();
      break;
    default:
      break;
  }
  return *this;
}

inline variable_t *variable_struct::operator *= (const variable_t *object) {
  if (object) *this = *object;
  return this;
}
  
template <typename T>
inline variable_t &variable_struct::operator *= (T value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  auto last = _get<T>();
  last *= value;
  data = std::to_string(last);
  return *this;
}

inline variable_t &variable_struct::operator /= (const variable_t &object) {
  switch (type) {
    case Int32:
      *this /= object.get<int32_t>();
      break;
    case Uint32:
      *this /= object.get<uint32_t>();
      break;
    case Int16:
      *this /= object.get<int16_t>();
      break;
    case Uint16:
      *this /= object.get<uint16_t>();
      break;
    case Int8:
      *this /= object.get<int8_t>();
      break;
    case Uint8:
      *this /= object.get<uint8_t>();
      break;
    case Int64:
      *this /= object.get<int64_t>();
      break;
    case Uint64:
      *this /= object.get<uint64_t>();
      break;
    case Float:
      *this /= object.get<float>();
      break;
    case Double:
      *this /= object.get<double>();
      break;
    default:
      break;
  }
  return *this;
}

inline variable_t *variable_struct::operator /= (const variable_t *object) {
  if (object) *this = *object;
  return this;
}

template <typename T>
inline variable_t &variable_struct::operator /= (T value) {
  std::unique_lock<std::mutex> auto_lock(mutex);
  auto last = _get<T>();
  last /= value;
  data = std::to_string(last);
  return *this;
}

inline variable_t &variable_struct::operator ++ () {
  *this += 1;
  return *this;
}
  
inline variable_t &variable_struct::operator -- () {
  *this -= 1;
  return *this;
}
  
inline variable_t &variable_struct::operator ++ (int32_t) {
  *this += 1;
  return *this;
}
  
inline variable_t &variable_struct::operator -- (int32_t) {
  *this -= 1;
  return *this;
}

inline bool variable_struct::operator == (const variable_t &object) const {
  return data == object.data;
}

inline bool variable_struct::operator == (const variable_t *object) const {
  if (object) return data == object->data;
  return false;
}
  
inline bool variable_struct::operator == (const std::string &value) const {
  return data == value;
}

inline bool variable_struct::operator == (const char *value) const {
  return data == value;
}
  
template <typename T>
inline bool variable_struct::operator == (T value) const {
  variable_t var{value};
  return *this == var;
}

inline bool variable_struct::operator != (const variable_t &object) const {
  return data != object.data;
}

inline bool variable_struct::operator != (const variable_t *object) const {
  if (object) return data != object->data;
  return true;
}

inline bool variable_struct::operator != (const std::string &value) const {
  return data != value;
}

inline bool variable_struct::operator != (const char *value) const {
  return data != value;
}
  
template <typename T>
inline bool variable_struct::operator != (T value) const {
  variable_t var{value};
  return *this != var;
}

inline bool variable_struct::operator < (const variable_t &object) const {
  return data < object.data;
}
  
inline bool variable_struct::operator < (const variable_t *object) const {
  if (object) return data < object->data;
  return false;
}
  
inline bool variable_struct::operator < (const std::string &value) const {
  return data < value;
}
 
inline bool variable_struct::operator < (const char *value) const {
  if (is_null(value)) return false;
  return data < value;
}
 
template <typename T>
inline bool variable_struct::operator < (T value) const {
  variable_t var{value};
  return *this < var;
}

inline bool variable_struct::operator > (const variable_t &object) const {
  return data > object.data;
}

inline bool variable_struct::operator > (const variable_t *object) const {
  if (object) return data > object->data;
  return true;
}
  
inline bool variable_struct::operator > (const std::string &value) const {
  return data > value;
}

inline bool variable_struct::operator > (const char *value) const {
  if (is_null(value)) return true;
  return data > value;
}
  
template <typename T>
inline bool variable_struct::operator > (T value) const {
  variable_t var{value};
  return *this > var;
}

inline variable_struct::operator const std::string() {
  return data;
}
  
inline variable_struct::operator const char *() {
  return data.c_str();
}
  
template <typename T>
inline variable_struct::operator T() {
  return this->get<T>();
}

inline std::ostream& operator<<(std::ostream& os, const variable_t &object) {
  using enum Variable;
  switch (object.type) {
    case Int32:
      os << object.get<int32_t>();
      break;
    case Uint32:
      os << object.get<uint32_t>();
      break;
    case Int16:
      os << object.get<int16_t>();
      break;
    case Uint16:
      os << object.get<uint16_t>();
      break;
    case Int8:
      os << object.get<int8_t>();
      break;
    case Uint8:
      os << object.get<uint8_t>();
      break;
    case Int64:
      os << object.get<int64_t>();
      break;
    case Uint64:
      os << object.get<uint64_t>();
      break;
    case Float:
      os << object.get<float>();
      break;
    case Double:
      os << object.get<double>();
      break;
    default:
      os << object.data;
      break;
  }
  return os;
}

} //namespace plain

#endif //PLAIN_BASIC_TYPE_VARIABLE_TCC_
