/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id logger.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 19:54
 * @uses The plain framework logger implemention.
 */
#ifndef PLAIN_BASIC_LOGGER_H_
#define PLAIN_BASIC_LOGGER_H_

#include "plain/basic/config.h"
#include "plain/basic/singleton.tcc"

namespace plain {

class PLAIN_API Logger : public Singleton<Logger> {

 public:
   Logger();
   ~Logger();

};

} // namespace plain

#endif // PLAIN_BASIC_LOGGER_H_
