#include "plain/basic/type/variable.h"
#include <iomanip>

namespace plain {

std::ostream &operator<<(std::ostream &os, const variable_t &object) noexcept {
  std::visit([&os](auto &&val){ os << val; }, object.value);
  return os;
}

std::ostream &operator<<(std::ostream &os, const bytes_t &bytes) noexcept {
  std::ios format {nullptr};
  format.copyfmt(os);
  if (bytes.empty()) {
    os << '0';
  } else {
    os << std::hex;
    for (const auto byte : bytes) {
      os << std::setfill('0')
         << std::setw(2)
         << std::uppercase
         << static_cast<unsigned>(byte);
    }
  }
  os.copyfmt(format);
  return os;
}

} // namespace plain
