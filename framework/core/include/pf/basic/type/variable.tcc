/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id variable.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/05/25 12:44
 * @uses The variable implement.
*/
#ifndef PF_BASIC_TYPE_VARIABLE_TCC_
#define PF_BASIC_TYPE_VARIABLE_TCC_

#include "pf/basic/type/variable.h"

namespace pf_basic {

namespace type {

template <typename T>
var_t std_convert_type(T) {
  if (is_same(bool, T)) return kVariableTypeBool;
  if (is_same(int32_t, T)) return kVariableTypeInt32;
  if (is_same(uint32_t, T)) return kVariableTypeUint32;
  if (is_same(int16_t, T)) return kVariableTypeInt16;
  if (is_same(uint16_t, T)) return kVariableTypeUint16;
  if (is_same(int8_t, T)) return kVariableTypeInt8;
  if (is_same(uint8_t, T)) return kVariableTypeUint8;
  if (is_same(int64_t, T)) return kVariableTypeInt64;
  if (is_same(uint64_t, T)) return kVariableTypeUint64;
  if (is_same(float, T)) return kVariableTypeFloat;
  if (is_same(double, T)) return kVariableTypeDouble;
  return kVariableTypeString; //Default is string.
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
  type = kVariableTypeString;
  data = value;
}

inline variable_struct::variable_struct(const char *value) {
  if (value) {
    type = kVariableTypeString;
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
  type = kVariableTypeString;
  data = value;
  return *this;
}

inline variable_t &variable_struct::operator = (const char *value) {
  if (is_null(value)) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = kVariableTypeString;
  data = value;
  return *this;
}
  
inline variable_t &variable_struct::operator = (char *value) {
  if (is_null(value)) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = kVariableTypeString;
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
    case kVariableTypeInt32:
      *this += object.get<int32_t>();
      break;
    case kVariableTypeUint32:
      *this += object.get<uint32_t>();
      break;
    case kVariableTypeInt16:
      *this += object.get<int16_t>();
      break;
    case kVariableTypeUint16:
      *this += object.get<uint16_t>();
      break;
    case kVariableTypeInt8:
      *this += object.get<int8_t>();
      break;
    case kVariableTypeUint8:
      *this += object.get<uint8_t>();
      break;
    case kVariableTypeInt64:
      *this += object.get<int64_t>();
      break;
    case kVariableTypeUint64:
      *this += object.get<uint64_t>();
      break;
    case kVariableTypeFloat:
      *this += object.get<float>();
      break;
    case kVariableTypeDouble:
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
  type = kVariableTypeString;
  data += value;
  return *this;
}

inline variable_t &variable_struct::operator += (const char *value) {
  if (is_null(value)) return *this;
  std::unique_lock<std::mutex> auto_lock(mutex);
  type = kVariableTypeString;
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
    case kVariableTypeInt32:
      *this -= object.get<int32_t>();
      break;
    case kVariableTypeUint32:
      *this -= object.get<uint32_t>();
      break;
    case kVariableTypeInt16:
      *this -= object.get<int16_t>();
      break;
    case kVariableTypeUint16:
      *this -= object.get<uint16_t>();
      break;
    case kVariableTypeInt8:
      *this -= object.get<int8_t>();
      break;
    case kVariableTypeUint8:
      *this -= object.get<uint8_t>();
      break;
    case kVariableTypeInt64:
      *this -= object.get<int64_t>();
      break;
    case kVariableTypeUint64:
      *this -= object.get<uint64_t>();
      break;
    case kVariableTypeFloat:
      *this -= object.get<float>();
      break;
    case kVariableTypeDouble:
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
    case kVariableTypeInt32:
      *this *= object.get<int32_t>();
      break;
    case kVariableTypeUint32:
      *this *= object.get<uint32_t>();
      break;
    case kVariableTypeInt16:
      *this *= object.get<int16_t>();
      break;
    case kVariableTypeUint16:
      *this *= object.get<uint16_t>();
      break;
    case kVariableTypeInt8:
      *this *= object.get<int8_t>();
      break;
    case kVariableTypeUint8:
      *this *= object.get<uint8_t>();
      break;
    case kVariableTypeInt64:
      *this *= object.get<int64_t>();
      break;
    case kVariableTypeUint64:
      *this *= object.get<uint64_t>();
      break;
    case kVariableTypeFloat:
      *this *= object.get<float>();
      break;
    case kVariableTypeDouble:
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
    case kVariableTypeInt32:
      *this /= object.get<int32_t>();
      break;
    case kVariableTypeUint32:
      *this /= object.get<uint32_t>();
      break;
    case kVariableTypeInt16:
      *this /= object.get<int16_t>();
      break;
    case kVariableTypeUint16:
      *this /= object.get<uint16_t>();
      break;
    case kVariableTypeInt8:
      *this /= object.get<int8_t>();
      break;
    case kVariableTypeUint8:
      *this /= object.get<uint8_t>();
      break;
    case kVariableTypeInt64:
      *this /= object.get<int64_t>();
      break;
    case kVariableTypeUint64:
      *this /= object.get<uint64_t>();
      break;
    case kVariableTypeFloat:
      *this /= object.get<float>();
      break;
    case kVariableTypeDouble:
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
  switch (object.type) {
    case kVariableTypeInt32:
      os << object.get<int32_t>();
      break;
    case kVariableTypeUint32:
      os << object.get<uint32_t>();
      break;
    case kVariableTypeInt16:
      os << object.get<int16_t>();
      break;
    case kVariableTypeUint16:
      os << object.get<uint16_t>();
      break;
    case kVariableTypeInt8:
      os << object.get<int8_t>();
      break;
    case kVariableTypeUint8:
      os << object.get<uint8_t>();
      break;
    case kVariableTypeInt64:
      os << object.get<int64_t>();
      break;
    case kVariableTypeUint64:
      os << object.get<uint64_t>();
      break;
    case kVariableTypeFloat:
      os << object.get<float>();
      break;
    case kVariableTypeDouble:
      os << object.get<double>();
      break;
    default:
      os << object.data;
      break;
  }
  return os;
}

} //namespace type

} //namespace pf_basic

#endif //PF_BASIC_TYPE_VARIABLE_TCC_
