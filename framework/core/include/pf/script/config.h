/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id config.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/01/09 20:26
 * @uses The script config file.
 *       对于各种类型的扩展，会在基本框架内实现常见的几种，外部可以扩展，考虑
 *       使用统一的接口模板，如cache模块一样，外部可以自由地扩展。
*/
#ifndef PF_SCRIPT_CONFIG_H_
#define PF_SCRIPT_CONFIG_H_

#include "pf/basic/config.h"
#include "pf/basic/type/config.h"

#if OS_WIN
#define SCRIPT_ROOT_PATH "public\\data\\script"
#define SCRIPT_WORK_PATH "\\"
#elif OS_UNIX
#define SCRIPT_ROOT_PATH "public/data/script"
#define SCRIPT_WORK_PATH "/"
#endif
#define SCRIPT_MODULENAME "script"

namespace pf_script {

class Interface;
class Factory;

typedef enum {
  kTypeLua = 0,   //Lua script.
  kTypeNumber,    //The script type number.
} type_t;

struct config_struct {
  std::string rootpath;
  std::string workpath;
  int8_t type;
};

using config_t = config_struct;
using eid_t = int16_t; //Environment.
/* Can't use in windows. */
//using var_array_t = pf_basic::type::variable_array_t;
//using var_t = pf_basic::type::variable_t;

}

#define SCRIPT_EID_INVALID (-1)

//Auto environment creator.
auto_envcreator(script, pf_script::Interface)

#endif //PF_SCRIPT_CONFIG_H_
