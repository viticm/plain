#include "plain/net/rpc/packer.h"

using plain::net::rpc::Packer;

Packer::Packer() = default;
Packer::~Packer() = default;

void Packer::pack(int8_t value) noexcept {
  if (value > 0 && value < std::to_underlying(Type::Max)) {
    serialized_object_.emplace_back(std::to_underlying(Type::Int8));
  }
  serialized_object_.emplace_back(uint8_t(twos_complement(value).to_ulong()));
}

void Packer::pack(int16_t value) noexcept {
  if (std::abs(value) < std::abs(std::numeric_limits<int8_t>::min())) {
    pack(static_cast<int8_t>(value));
  } else {
    serialized_object_.emplace_back(std::to_underlying(Type::Int16));
    auto serialize_value = 
      static_cast<uint16_t>(twos_complement(value).to_ulong());
    for (auto i = sizeof(value); i > 0; --i) {
      serialized_object_.emplace_back(
        static_cast<uint8_t>(serialize_value >> (8U * (i - 1)) & 0xff));
    }
  }
}

void Packer::pack(int32_t value) noexcept {
  if (std::abs(value) < std::abs(std::numeric_limits<int16_t>::min())) {
    pack(static_cast<int16_t>(value));
  } else {
    serialized_object_.emplace_back(std::to_underlying(Type::Int32));
    auto serialize_value = 
      static_cast<uint32_t>(twos_complement(value).to_ulong());
    for (auto i = sizeof(value); i > 0; --i) {
      serialized_object_.emplace_back(
        static_cast<uint8_t>(serialize_value >> (8U * (i - 1)) & 0xff));
    }
  }
}

void Packer::pack(int64_t value) noexcept {
  if (std::abs(value) < std::abs(std::numeric_limits<int32_t>::min()) &&
      value != std::numeric_limits<int64_t>::min()) {
    pack(static_cast<int32_t>(value));
  } else {
    serialized_object_.emplace_back(std::to_underlying(Type::Int64));
    auto serialize_value = 
      static_cast<uint64_t>(twos_complement(value).to_ulong());
    for (auto i = sizeof(value); i > 0; --i) {
      serialized_object_.emplace_back(
        static_cast<uint8_t>(serialize_value >> (8U * (i - 1)) & 0xff));
    }
  }
}

void Packer::pack(uint8_t value) noexcept {
  if (value <= 0x7f) {
    serialized_object_.emplace_back(value);
  } else {
    serialized_object_.emplace_back(std::to_underlying(Type::Uint8));
    serialized_object_.emplace_back(value);
  }
}

void Packer::pack(uint16_t value) noexcept {
  if (value > std::numeric_limits<uint8_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Uint16));
    for (auto i = sizeof(value); i > 0U; --i) {
      serialized_object_.emplace_back(
        static_cast<uint8_t>(value >> (8U * (i - 1)) & 0xff));
    }
  } else {
    pack(static_cast<uint8_t>(value));
  }
}

void Packer::pack(uint32_t value) noexcept {
  if (value > std::numeric_limits<uint16_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Uint32));
    for (auto i = sizeof(value); i > 0U; --i) {
      serialized_object_.emplace_back(
        static_cast<uint8_t>(value >> (8U * (i - 1)) & 0xff));
    }
  } else {
    pack(static_cast<uint16_t>(value));
  }
}

void Packer::pack(uint64_t value) noexcept {
  if (value > std::numeric_limits<uint32_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Uint64));
    for (auto i = sizeof(value); i > 0U; --i) {
      serialized_object_.emplace_back(
        static_cast<uint8_t>(value >> (8U * (i - 1)) & 0xff));
    }
  } else {
    pack(static_cast<uint32_t>(value));
  }
}

void Packer::pack(std::nullptr_t) noexcept {
  serialized_object_.emplace_back(std::to_underlying(Type::Null));
}

void Packer::pack(bool value) noexcept {
  if (value)
    serialized_object_.emplace_back(std::to_underlying(Type::TrueBool));
  else
    serialized_object_.emplace_back(std::to_underlying(Type::FalseBool));
}

void Packer::pack(float value) noexcept {
  double integral_part{.0};
  auto fractional_remainder = float(std::modf(value, &integral_part));

  if (fractional_remainder == 0) { // Just pack as int
    pack(int64_t(integral_part));
  } else {
    static_assert(std::numeric_limits<float>::radix == 2);
    auto exponent = std::ilogb(value);
    float full_mantissa = value / float(scalbn(1.0, exponent));
    auto sign_mask = std::bitset<32>(
      static_cast<uint32_t>(std::signbit(full_mantissa)) << 31);
    auto excess_127_exponent_mask =
      std::bitset<32>(uint32_t(exponent + 127) << 23);
    auto normalized_mantissa_mask = std::bitset<32>();
    float implied_mantissa = std::fabs(full_mantissa) - 1.0f;
    for (auto i = 23U; i > 0; --i) {
      integral_part = 0;
      implied_mantissa *= 2;
      implied_mantissa = float(std::modf(implied_mantissa, &integral_part));
      if (uint8_t(integral_part) == 1) {
        normalized_mantissa_mask |= std::bitset<32>(uint32_t(1 << (i - 1)));
      }
    }

    uint32_t ieee754_float32 = (sign_mask | excess_127_exponent_mask |
       normalized_mantissa_mask).to_ulong();
    serialized_object_.emplace_back(std::to_underlying(Type::Float32));
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      serialized_object_.emplace_back(
        uint8_t(ieee754_float32 >> (8U * (i - 1)) & 0xff));
    }
  }
}

void Packer::pack(double value) noexcept {
  double integral_part{.0};
  double fractional_remainder = std::modf(value, &integral_part);

  if (fractional_remainder == 0) { // Just pack as int
    pack(int64_t(integral_part));
  } else {
    static_assert(std::numeric_limits<float>::radix == 2);
    auto exponent = std::ilogb(value);
    double full_mantissa = value / std::scalbn(1.0, exponent);
    auto sign_mask =
      std::bitset<64>(uint64_t(std::signbit(full_mantissa)) << 63);
    auto excess_127_exponent_mask =
      std::bitset<64>(uint64_t(exponent + 1023) << 52);
    auto normalized_mantissa_mask = std::bitset<64>();
    double implied_mantissa = std::fabs(full_mantissa) - 1.0f;

    for (auto i = 52U; i > 0; --i) {
      integral_part = 0;
      implied_mantissa *= 2;
      implied_mantissa = modf(implied_mantissa, &integral_part);
      if (uint8_t(integral_part) == 1) {
        normalized_mantissa_mask |= std::bitset<64>(uint64_t(1) << (i - 1));
      }
    }
    auto ieee754_float64 = 
      (sign_mask | excess_127_exponent_mask |
       normalized_mantissa_mask).to_ullong();
    serialized_object_.emplace_back(std::to_underlying(Type::Float64));
    for (auto i = sizeof(ieee754_float64); i > 0; --i) {
      serialized_object_.emplace_back(
        uint8_t(ieee754_float64 >> (8U * (i - 1)) & 0xff));
    }
  }
}

void Packer::pack(const std::string &value) noexcept {
  if (value.size() < 32) {
    serialized_object_.emplace_back(uint8_t(value.size()) | 0b10100000);
  } else if (value.size() < std::numeric_limits<uint8_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Str8));
    serialized_object_.emplace_back(uint8_t(value.size()));
  } else if (value.size() < std::numeric_limits<uint16_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Str16));
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      serialized_object_.emplace_back(
        uint8_t(value.size() >> (8U * (i - 1)) & 0xff));
    }
  } else if (value.size() < std::numeric_limits<uint32_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Str32));
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      serialized_object_.emplace_back(
        uint8_t(value.size() >> (8U * (i - 1)) & 0xff));
    }
  } else {
    return; // Give up if string is too long
  }

  for (char i : value) {
    serialized_object_.emplace_back(static_cast<uint8_t>(i));
  }
}

void Packer::pack(const std::vector<uint8_t> &value) noexcept {
  if (value.size() < std::numeric_limits<uint8_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Bin8));
    serialized_object_.emplace_back(uint8_t(value.size()));
  } else if (value.size() < std::numeric_limits<uint16_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Bin16));
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      serialized_object_.emplace_back(
        uint8_t(value.size() >> (8U * (i - 1)) & 0xff));
    }
  } else if (value.size() < std::numeric_limits<uint32_t>::max()) {
    serialized_object_.emplace_back(std::to_underlying(Type::Bin32));
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      serialized_object_.emplace_back(
        uint8_t(value.size() >> (8U * (i - 1)) & 0xff));
    }
  } else {
    return; // Give up if vector is too large
  }

  for (const auto &elem : value) {
    serialized_object_.emplace_back(elem);
  }
}
