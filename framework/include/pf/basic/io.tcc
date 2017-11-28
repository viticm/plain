/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id io.tcc
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2016/05/06 23:04
 * @uses The base io functions.
 *       cerr: red font.
 *       cwarn: yellow font.
 *       cdebug: green font.
*/
#ifndef PF_BASIC_IO_TCC_
#define PF_BASIC_IO_TCC_

#include "pf/basic/config.h"

namespace pf_basic {

#if OS_WIN
inline uint16_t set_consolecolor(uint16_t forecolor = 0, 
                                 uint16_t background_color = 0) {
  CONSOLE_SCREEN_BUFFER_INFO consolescreen_bufferinfo;
  if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), 
                                  &consolescreen_bufferinfo)) {
    return 0;
  }
  if (!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
                               forecolor | background_color)) {
    return 0;
  }
  return consolescreen_bufferinfo.wAttributes;
}

inline void reset_consolecolor(uint16_t color) {
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
#endif


template<typename... TS> 
void io_cerr(const char *format, TS... args) {
#if OS_UNIX
  std::cout << "\e[0;31;1m";  
#elif OS_WIN
  auto lastcolor = set_consolecolor(FOREGROUND_INTENSITY | FOREGROUND_RED);
#endif
  printf(format, args...);
#if OS_UNIX
  std::cout << "\e[0m";
#endif
  std::cout << std::endl;
#if OS_WIN
  reset_consolecolor(lastcolor);
#endif
};

template<typename... TS> 
void io_cwarn(const char *format, TS... args) {
#if OS_UNIX
  std::cout<<"\e[0;33;1m";  
#elif OS_WIN
  auto lastcolor = 
    set_consolecolor(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
#endif
  printf(format, args...);
#if OS_UNIX
  std::cout << "\e[0m";
#endif
  std::cout << std::endl;
#if OS_WIN
  reset_consolecolor(lastcolor);
#endif
};

template<typename... TS> 
void io_cdebug(const char *format, TS... args) {
#if OS_UNIX
  std::cout << "\e[0;32;1m";  
#elif OS_WIN
  auto lastcolor = set_consolecolor(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
#endif
  printf(format, args...);
#if OS_UNIX
  std::cout << "\e[0m";
#endif
  std::cout << std::endl;
#if OS_WIN
  reset_consolecolor(lastcolor);
#endif
};

}; //namespace pf_basic

#endif //PF_BASIC_IO_TCC_
