/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id base64.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2018/12/29 11:23
 * @uses The base64 function.
*/
//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//

#ifndef PF_BASIC_BASE64_H_
#define PF_BASIC_BASE64_H_

#include "pf/basic/config.h" 

namespace pf_basic {

PF_API std::string base64_encode(unsigned char const* , unsigned int len);
PF_API std::string base64_decode(std::string const& s);

} //namespace pf_basic

#endif /* PF_BASIC_BASE64_H_ */
