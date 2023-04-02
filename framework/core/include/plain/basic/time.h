/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id time.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 19:56
 * @uses The plain framework time class.
 */

#ifndef PLAIN_BASIC_TIME_H_
#define PLAIN_BASIC_TIME_H_

#include "plain/basic/config.h"
#include "plain/basic/singleton.tcc"

namespace plain {

class PLAIN_API Time : public Singleton<Time> {

 public:
   Time();
   ~Time();

 public:
   
   // Get tick count from start.
   uint64_t get_tick() const noexcept;
   
   // Get unix timestamp.
   static uint32_t get_timestamp();
   
   // Get format string like{Y-m-d H:M:S}
   static std::string format();
   
   // Get format string like{Y-m-d H:M:S[.ms]}
   static std::string format(bool show_microseconds = true);

 private:
   std::chrono::time_point<std::chrono::steady_clock> s_time_;

};

} // namespace plain

#endif // PLAIN_BASIC_TIME_H_
