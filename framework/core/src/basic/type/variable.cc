#include "plain/basic/type/variable.h"

namespace plain {

variable_t&
variable_struct::operator+=(const variable_t& object) noexcept {
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

variable_t&
variable_struct::operator-=(const variable_t& object) noexcept {
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

variable_t& 
variable_struct::operator*=(const variable_t& object) noexcept {
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

inline variable_t& 
variable_struct::operator/=(const variable_t& object) noexcept {
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

std::ostream& operator<<(std::ostream& os, const variable_t& object) {
  using enum Variable;
  os << "plain::variable: ";
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

}
