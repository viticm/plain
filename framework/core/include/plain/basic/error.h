/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id error.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/11/07 22:11
 * @uses The error class implemention.
 */

#ifndef PLAIN_BASIC_ERROR_H_
#define PLAIN_BASIC_ERROR_H_

#include "plain/basic/config.h"
#include "plain/basic/concepts.h"
#include <format>
#include <variant>
#include <utility>

namespace plain {

// Inner codes.
enum class ErrorCode : std::uint8_t {
  None = 0,
  RunTime,
  NetPacketCantFill,
  NetPacketNeedRecv,
  NetPacketInvalid,
  OutOfRange,
};

class PLAIN_API [[nodiscard]] Error : copyable {

 public:
  using code_t = int32_t;

 public:

  Error() {

  }

  Error(code_t code, std::string_view message = {}) :
    code_{code}, message_{message} {

  }

  Error(enums auto code, std::string_view message = {}) :
    code_{std::to_underlying(code)}, message_{message} {

  }

  template <typename ...Args>
  Error(code_t code, std::string_view format, Args ...args) :
    Error(code, std::vformat(format, std::move(args)...)) {

  }

  template <typename ...Args>
  Error(enums auto code, std::string_view format, Args ...args) :
    Error(std::to_underlying(code), std::vformat(format, std::move(args)...)) {

  }

 public:
  bool operator==(const Error &) const noexcept = default;
  bool operator!=(const Error &) const noexcept = default;
  bool valid() const noexcept {
    return code_ != std::to_underlying(ErrorCode::None);
  }
  explicit operator bool() const noexcept {
    return valid();
  }
  explicit operator ErrorCode() const noexcept {
    return static_cast<ErrorCode>(code_);
  }

 public:
  friend std::ostream &operator<<(std::ostream &os, const Error &e) {
    return os << std::to_string(e.code()) << std::string(": ") << e.message();
  }

  friend bool operator==(const Error &e, ErrorCode code) noexcept {
    return e.code() == std::to_underlying(code);
  }

 public:
  code_t code() const noexcept {
    return code_;
  }

  const std::string &message() const noexcept {
    return message_;
  }

  std::string to_string() const noexcept {
    std::stringstream ss;
    ss << *this;
    return ss.str();
  }

  void set_code(code_t code) noexcept {
    code_ = code;
  }

  void set_message(std::string_view message) noexcept {
    message_ = message;
  }

  bool is_code(enums auto e) {
    return std::to_underlying(e) == code_;
  }

 private:
  code_t code_{std::to_underlying(ErrorCode::None)};
  std::string message_;

};

template <typename T>
using error_or_t = std::variant<T, Error>;

template <typename T>
Error *get_error(error_or_t<T> &maybe_error) {
  return std::get_if<Error>(&maybe_error);
}

template <typename T>
const Error *get_error(const error_or_t<T> &maybe_error) {
  return std::get_if<Error>(&maybe_error);
}

} // namespace plain

#endif // PLAIN_BASIC_ERROR_H_
