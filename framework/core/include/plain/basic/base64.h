/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id base64.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 17:57
 * @uses your description
 */
#ifndef PLAIN_BASIC_BASE64_H_
#define PLAIN_BASIC_BASE64_H_

#include "plain/basic/config.h" 

namespace plain {

PLAIN_API std::string base64_encode(unsigned char const* , unsigned int len);
PLAIN_API std::string base64_decode(std::string const& s);

} // namespace plain

#endif /* PLAIN_BASIC_BASE64_H_ */
