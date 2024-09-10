/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id copyable.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 20:06
 * @uses The copyable class.
 */

#ifndef PLAIN_BASIC_COPYABLE_H_
#define PLAIN_BASIC_COPYABLE_H_

namespace plain {

class copyable {

 protected:
   copyable() noexcept = default;
   ~copyable() noexcept = default;
   copyable(const copyable &) noexcept = default;
   copyable(copyable &&) noexcept = default;
   copyable &operator=(copyable &&) noexcept = default;
   copyable &operator=(const copyable &) noexcept = default;
   bool operator==(const copyable &) const noexcept = default;

};

} // namespace plain

#endif // PLAIN_BASIC_COPYABLE_H_
