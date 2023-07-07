/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id noncopyable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 20:28
 * @uses The noncopyable class.
 */

#ifndef PLAIN_BASIC_NONCOPYABLE_H_
#define PLAIN_BASIC_NONCOPYABLE_H_

namespace plain {

class noncopyable {

 public:
   noncopyable(const noncopyable &) = delete;
   noncopyable &operator = (const noncopyable &) = delete;

 protected:
   noncopyable() noexcept = default;
   ~noncopyable() noexcept = default;
   noncopyable(noncopyable &&) noexcept = default;
   noncopyable &operator = (noncopyable &&) noexcept = default;

};

} // namespace plain

#endif // PLAIN_BASIC_NONCOPYABLE_H_
