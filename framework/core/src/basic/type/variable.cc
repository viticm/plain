#include "plain/basic/type/variable.h"

namespace plain {

std::ostream& operator<<(std::ostream& os, const variable_t& object) {
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

}
