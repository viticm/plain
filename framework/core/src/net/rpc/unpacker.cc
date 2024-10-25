#include "plain/net/rpc/unpacker.h"
#include <bitset>
#include <cmath>

using plain::net::rpc::Unpacker;

Unpacker::Unpacker() = default;

Unpacker::Unpacker(const uint8_t *data, size_t size) :
  data_{data}, data_end_{data + size} {

}

Unpacker::~Unpacker() = default;

void Unpacker::unpack(int8_t &value) noexcept {
  if (data() == std::to_underlying(Type::Int8)) {
    increment();
    value = data();
    increment();
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(int16_t &value) noexcept {
  if (data() == std::to_underlying(Type::Int16)) {
    increment();
    std::bitset<16> bits;
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      bits |= uint16_t(data()) << 8 * (i - 1);
      increment();
    }
    if (bits[15]) {
      value = -1 * (uint16_t((~bits).to_ulong()) + 1);
    } else {
      value = uint16_t(bits.to_ulong());
    }
  } else if (data() == std::to_underlying(Type::Int8)) {
    int8_t val;
    unpack(val);
    value = val;
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(int32_t &value) noexcept {
  if (data() == std::to_underlying(Type::Int32)) {
    increment();
    std::bitset<32> bits;
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      bits |= uint32_t(data()) << 8 * (i - 1);
      increment();
    }
    if (bits[31]) {
      value = -1 * ((~bits).to_ulong() + 1);
    } else {
      value = bits.to_ulong();
    }
  } else if (data() == std::to_underlying(Type::Int16)) {
    int16_t val;
    unpack(val);
    value = val;
  } else if (data() == std::to_underlying(Type::Int8)) {
    int8_t val;
    unpack(val);
    value = val;
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(int64_t &value) noexcept {
  if (data() == std::to_underlying(Type::Int64)) {
    increment();
    std::bitset<64> bits;
    for (auto i = sizeof(value); i > 0; --i) {
      bits |= std::bitset<8>(data()).to_ullong() << 8 * (i - 1);
      increment();
    }
    if (bits[63]) {
      value = -1 * ((~bits).to_ullong() + 1);
    } else {
      value = bits.to_ullong();
    }
  } else if (data() == std::to_underlying(Type::Int32)) {
    int32_t val;
    unpack(val);
    value = val;
  } else if (data() == std::to_underlying(Type::Int16)) {
    int16_t val;
    unpack(val);
    value = val;
  } else if (data() == std::to_underlying(Type::Int8)) {
    int8_t val;
    unpack(val);
    value = val;
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(uint8_t &value) noexcept {
  if (data() == std::to_underlying(Type::Uint8)) {
    increment();
    value = data();
    increment();
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(uint16_t &value) noexcept {
  if (data() == std::to_underlying(Type::Uint16)) {
    increment();
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      value += data() << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Uint8)) {
    increment();
    value = data();
    increment();
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(uint32_t &value) noexcept {
  if (data() == std::to_underlying(Type::Uint32)) {
    increment();
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      value += data() << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Uint16)) {
    increment();
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      value += data() << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Uint8)) {
    increment();
    value = data();
    increment();
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(uint64_t &value) noexcept {
  if (data() == std::to_underlying(Type::Uint64)) {
    increment();
    for (auto i = sizeof(uint64_t); i > 0; --i) {
      value += uint64_t(data()) << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Uint32)) {
    increment();
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      value += uint64_t(data()) << 8 * (i - 1);
      increment();
    }
    data_++;
  } else if (data() == std::to_underlying(Type::Uint16)) {
    increment();
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      value += uint64_t(data()) << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Uint8)) {
    increment();
    value = data();
    increment();
  } else {
    value = data();
    increment();
  }
}

void Unpacker::unpack(std::nullptr_t &) noexcept {
  increment();
}

void Unpacker::unpack(bool &value) noexcept {
  value = data() == std::to_underlying(Type::TrueBool);
  increment();
}

void Unpacker::unpack(float &value) noexcept {
  if (data() == std::to_underlying(Type::Float32)) {
    increment();
    uint32_t _data = 0;
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      _data += data() << 8 * (i - 1);
      increment();
    }
    auto bits = std::bitset<32>(_data);
    auto mantissa = 1.0f;
    for (auto i = 23U; i > 0; --i) {
      if (bits[i - 1]) {
        mantissa += 1.0f / (1 << (24 - i));
      }
    }
    if (bits[31]) {
      mantissa *= -1;
    }
    uint8_t exponent = 0;
    for (auto i = 0U; i < 8; ++i) {
      exponent += bits[i + 23] << i;
    }
    exponent -= 127;
    value = std::ldexp(mantissa, exponent);
  } else {
    if (data() == std::to_underlying(Type::Int8) ||
        data() == std::to_underlying(Type::Int16) ||
        data() == std::to_underlying(Type::Int32) ||
        data() == std::to_underlying(Type::Int64)) {
      int64_t val = 0;
      unpack(val);
      value = float(val);
    } else {
      uint64_t val = 0;
      unpack(val);
      value = float(val);
    }
  }
}

void Unpacker::unpack(double &value) noexcept {
  if (data() == std::to_underlying(Type::Float64)) {
    increment();
    uint64_t _data = 0;
    for (auto i = sizeof(uint64_t); i > 0; --i) {
      _data += uint64_t(data()) << 8 * (i - 1);
      increment();
    }
    auto bits = std::bitset<64>(_data);
    auto mantissa = 1.0;
    for (auto i = 52U; i > 0; --i) {
      if (bits[i - 1]) {
        mantissa += 1.0 / (uint64_t(1) << (53 - i));
      }
    }
    if (bits[63]) {
      mantissa *= -1;
    }
    uint16_t exponent = 0;
    for (auto i = 0U; i < 11; ++i) {
      exponent += bits[i + 52] << i;
    }
    exponent -= 1023;
    value = ldexp(mantissa, exponent);
  } else {
    if (data() == std::to_underlying(Type::Int8) ||
        data() == std::to_underlying(Type::Int16) ||
        data() == std::to_underlying(Type::Int32) ||
        data() == std::to_underlying(Type::Int64)) {
      int64_t val = 0;
      unpack(val);
      value = float(val);
    } else {
      uint64_t val = 0;
      unpack(val);
      value = float(val);
    }
  }
}

void Unpacker::unpack(std::string &value) noexcept {
  std::size_t str_size = 0;
  if (data() == std::to_underlying(Type::Str32)) {
    increment();
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      str_size += uint32_t(data()) << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Str16)) {
    increment();
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      str_size += uint16_t(data()) << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Str8)) {
    increment();
    for (auto i = sizeof(uint8_t); i > 0; --i) {
      str_size += uint8_t(data()) << 8 * (i - 1);
      increment();
    }
  } else {
    str_size = data() & 0b00011111;
    increment();
  }
  if (data_ + str_size <= data_end_) {
    value = std::string{data_, data_ + str_size};
    increment(str_size);
  } else {
    error_ = ErrorCode::OutOfRange;
  }
}

void Unpacker::unpack(std::vector<uint8_t> &value) noexcept {
  std::size_t bin_size = 0;
  if (data() == std::to_underlying(Type::Bin32)) {
    increment();
    for (auto i = sizeof(uint32_t); i > 0; --i) {
      bin_size += uint32_t(data()) << 8 * (i - 1);
      increment();
    }
  } else if (data() == std::to_underlying(Type::Bin16)) {
    increment();
    for (auto i = sizeof(uint16_t); i > 0; --i) {
      bin_size += uint16_t(data()) << 8 * (i - 1);
      increment();
    }
  } else {
    increment();
    for (auto i = sizeof(uint8_t); i > 0; --i) {
      bin_size += uint8_t(data()) << 8 * (i - 1);
      increment();
    }
  }
  if (data_ + bin_size <= data_end_) {
    value = std::vector<uint8_t>{data_, data_ + bin_size};
    increment(bin_size);
  } else {
    error_ = ErrorCode::OutOfRange;
  }
}
