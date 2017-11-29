/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/06 23:38
 * @uses the base hashmap module config
 */
#ifndef PF_BASIC_HASHMAP_CONFIG_H_
#define PF_BASIC_HASHMAP_CONFIG_H_

#include "pf/basic/config.h"
#undef max
#undef min
#include <unordered_map>
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#endif //PF_BASIC_HASHMAP_CONFIG_H_
