/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id utility.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2023/03/31 22:53
 * @uses The plain utility functions.
 */
#ifndef PLAIN_BASIC_UTILITY_H_
#define PLAIN_BASIC_UTILITY_H_

#include "plain/basic/config.h"

namespace plain {

PLAIN_API std::string format_size(uint64_t size);

PLAIN_API char get_base64char(int32_t index);

PLAIN_API void replace_all(std::string& str, 
                           const std::string source, 
                           const std::string destination);

PLAIN_API int64_t toint64(const char* str);
PLAIN_API uint64_t touint64(const char* str);

//A simple de/encrypt for password.
PLAIN_API bool encrypt(const std::string& in, std::string& out);
PLAIN_API bool decrypt(const std::string& in, std::string& out);

//A simple de/encrypt string with a number.
PLAIN_API bool encrypt(const std::string& in, int32_t number, std::string& out);
PLAIN_API bool decrypt(const std::string& in, int32_t &number, std::string& out);

PLAIN_API char* safecopy(char* dest, const char* src, size_t size);

PLAIN_API int32_t charset_convert(const char* from, 
                                  const char* to, 
                                  char* save, 
                                  int32_t save_length, 
                                  const char* src, 
                                  int32_t src_length) ;

PLAIN_API bool get_escapechar(char in, char &out);
PLAIN_API bool getescape(const char* in, size_t insize, char* out, size_t outsize);
PLAIN_API int32_t explode(const char* source,
                          std::vector<std::string> &result,
                          const char* key,
                          bool one_key,
                          bool ignore_empty);
PLAIN_API bool checkstr(const char* in, uint32_t size);

// Retira espacios en blanco (u otros caracteres) del inicio de un string.
PLAIN_API std::string& ltrim(
    std::string& str, const std::string& character_mask = " \t\n\r\0\x0B");

// Retira los espacios en blanco (u otros caracteres) del final de un string.
PLAIN_API std::string& rtrim(
    std::string& str, const std::string& character_mask = " \t\n\r\0\x0B");

// Elimina espacio en blanco (u otro tipo de caracteres) del inicio y el final de la cadena.
PLAIN_API std::string& trim(
    std::string& str, const std::string& character_mask = " \t\n\r\0\x0B");

// Determine if a given string contains a given substring.
PLAIN_API bool contains(
    const std::string& haystack, const std::vector<std::string> &needles);

// Reemplaza todas las apariciones del string buscado con el string de reemplazo.
PLAIN_API std::string str_replace(const std::string& search , 
                               const std::string& replace, 
                               const std::string& subject, 
                               int32_t count = -1);

// Reemplaza todas las apariciones del string buscado con el string de reemplazo.
PLAIN_API std::string str_replaces(const std::vector<std::string> &search , 
                                const std::string& replace, 
                                const std::string& subject, 
                                int32_t count = -1);
// Quote string with slashes.
PLAIN_API inline std::string addslashes(const std::string& str) {
  return str_replace(
      "'", "\\'", str_replace("\"", "\\\"", str_replace("\\", "\\\\", str)));
}

// Quote string without slashes.
PLAIN_API inline std::string stripslashes(const std::string& str) {
  return str_replace(
      "\\'", "'", str_replace("\\\"", "\"", str_replace("\\\\", "\\", str)));
}

#if OS_WIN
# pragma warning( push )
# pragma warning( disable: 4244 )
#endif

// To lower.
PLAIN_API inline std::string tolower(const std::string& str) {
  std::string temp{ str };
  std::transform(
      temp.begin(), temp.end(), temp.begin(), (int32_t (*)(int32_t))std::tolower);
  return temp;
}

// To upper.
PLAIN_API inline std::string toupper(const std::string& str) {
  std::string temp{ str };
  std::transform(
      temp.begin(), temp.end(), temp.begin(), (int32_t (*)(int32_t))std::toupper);
  return temp;
}

PLAIN_API std::string wstr2str(const std::wstring& str);
PLAIN_API std::wstring str2wstr(const std::string& str);

#if OS_WIN
# pragma warning( pop )
#endif

PLAIN_API char value_toascii(char in);
PLAIN_API char ascii_tovalue(char in);
PLAIN_API bool binary_tostring(const char* in, uint32_t in_length, char* out);
PLAIN_API bool string_tobinary(const char* in, 
                               uint32_t in_length, 
                               char* out, 
                               uint32_t out_limit, 
                               uint32_t &out_length);
PLAIN_API void sleep(uint32_t million_seconds);

PLAIN_API void path_tounix(char* buffer, uint16_t length);
PLAIN_API void path_towindows(char* buffer, uint16_t length); 

//获取当前执行文件名
PLAIN_API void get_module_filename(char* buffer, size_t size);

PLAIN_API void disable_windowclose();

PLAIN_API bool makedir(const char* path, uint16_t mode = 755);

PLAIN_API uint32_t get_highsection(uint64_t value);

PLAIN_API uint32_t get_lowsection(uint64_t value);

PLAIN_API void dirname(const char* filepath, char* save);
PLAIN_API void complementpath(char* filepath, size_t size, char delimiter = '/');

PLAIN_API uint64_t touint64(uint32_t high, uint32_t low);

PLAIN_API char* strerror_pl(int32_t saved_errno);

} // namespace plain

#endif // PLAIN_BASIC_UTILITY_H_
