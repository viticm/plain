/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id function.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/04 22:43
 * @uses The base define function macros.
*/
#ifndef PLAIN_BASIC_MACROS_FUNCTION_H_
#define PLAIN_BASIC_MACROS_FUNCTION_H_

#include "pf/basic/macros/platform.h"

//c output/string functions
#if OS_WIN
#ifndef snprintf
#define snprintf _snprintf
#endif
#ifndef stricmp
#define stricmp _stricmp
#endif
#ifndef strnicmp
#define strnicmp _strnicmp
#endif
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif
#define strtoint64(pointer,endpointer,base) \
  _strtoi64(pointer,endpointer,base)
#define strtouint64(pointer,endpointer,base) \
  _strtoui64(pointer,endpointer,base)
#elif OS_UNIX
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#define strtoint64(pointer,endpointer,base) strtoll(pointer,endpointer,base)
#define strtouint64(pointer,endpointer,base) strtoull(pointer,endpointer,base)
#endif

#if OS_WIN
#define access _access
#define mkdir(dir,mode) _mkdir(dir)
#endif

#ifndef is_null
#define is_null(x) (nullptr == (x))
#endif

#define POINTER_TOINT64(pointer) \
  (int64_t)(reinterpret_cast<int64_t *>(pointer))
#define INT64_TOPOINTER(type,number) \
  reinterpret_cast<type *>((int64_t *)(number))

#define safe_delete(ptr) if (ptr) { delete ptr; ptr = nullptr; }
#define safe_delete_array(ptr) if (ptr) { delete[] ptr; ptr = nullptr; }

/* macro to avoid warnings about unused variables */
#if !defined(UNUSED)
#define UNUSED(x) ((void)(x))
#endif

/* Move a pointer to the std::unique_ptr value. */
#ifndef unique_move
#define unique_move(t,o,n) {std::unique_ptr< t > p{o}; n = std::move(p);}
#endif

/* C cast pointer function. */
#ifndef cast
#define cast(t, exp)((t)(exp))
#endif

/* C++ return the variable is the same type. */
#ifndef is_type
#define is_type(t,v) std::is_same<decltype((v)), (t)>::value
#endif

/* C++ stand useful macro. */
#ifndef is_same
#define is_same(t1,t2) (std::is_same<t1, t2>::value)
#endif

/* All env creator in pf macro implemention. */
#ifndef auto_envcreator
#define auto_envcreator(name,interface) \
  typedef interface * (__stdcall *function_env_creator_##name)(); \
  inline std::unordered_map<uint8_t, function_env_creator_##name> & \
    get_env_creator_map_##name() { \
    static std::unordered_map<uint8_t, function_env_creator_##name> creatormap; \
    return creatormap; \
  } \
  inline void register_env_creator_##name(uint8_t type, \
                                          function_env_creator_##name func) { \
    get_env_creator_map_##name()[type] = func; \
  } \
  inline function_env_creator_##name get_env_creator_##name(uint8_t type) { \
    return get_env_creator_map_##name()[type]; \
  }
#endif

/* C++ instanceof */
#ifndef instanceof
#define instanceof(p,c) (dynamic_cast<c *>(p) != nullptr)
#endif

#endif //PLAIN_BASIC_MACROS_FUNCTION_H_
