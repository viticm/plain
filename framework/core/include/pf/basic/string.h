/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id string.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2014/06/18 13:45
 * @uses base string module
 */
#ifndef PF_BASIC_STRING_H_
#define PF_BASIC_STRING_H_

#include <map> 
#include "pf/basic/config.h"

namespace pf_basic {

namespace string {

PF_API char get_base64char(int index);

PF_API void replace_all(std::string &str, 
                        const std::string source, 
                        const std::string destination);

PF_API bool toint16(const char *source, 
                    int16_t &result, 
                    uint8_t converted_length = 0, 
                    bool ignored_zero = false);

PF_API bool toint32(const char *source, 
                    int32_t &result, 
                    uint8_t converted_length = 0, 
                    bool ignored_zero = false);
PF_API int64_t toint64(const char *str);
PF_API uint64_t touint64(const char *str);

PF_API void encrypt(const char *in, char *out, int32_t out_length);

PF_API void decrypt(const char *in, char *out, int32_t out_length);

//A simple de/encrypt for password.
PF_API bool encrypt(const std::string &in, std::string &out);
PF_API bool decrypt(const std::string &in, std::string &out);

PF_API char *safecopy(char *dest, const char *src, size_t size);

PF_API int32_t charset_convert(const char *from, 
                               const char *to, 
                               char *save, 
                               int32_t save_length, 
                               const char *src, 
                               int32_t src_length) ;

PF_API bool get_escapechar(char in, char &out);
PF_API bool getescape(const char *in, size_t insize, char *out, size_t outsize);
PF_API int32_t explode(const char *source,
                       std::vector<std::string> &result,
                       const char *key,
                       bool one_key,
                       bool ignore_empty);
PF_API bool checkstr(const char *in, uint32_t size);

// Retira espacios en blanco (u otros caracteres) del inicio de un string.
PF_API std::string &ltrim(
    std::string &str, const std::string &character_mask = " \t\n\r\0\x0B");

// Retira los espacios en blanco (u otros caracteres) del final de un string.
PF_API std::string &rtrim(
    std::string &str, const std::string &character_mask = " \t\n\r\0\x0B");

// Elimina espacio en blanco (u otro tipo de caracteres) del inicio y el final de la cadena.
PF_API std::string &trim(
    std::string &str, const std::string &character_mask = " \t\n\r\0\x0B");

// Determine if a given string contains a given substring.
PF_API bool contains(
    const std::string &haystack, const std::vector<std::string> &needles);

// Reemplaza todas las apariciones del string buscado con el string de reemplazo.
PF_API std::string str_replace(const std::string &search , 
                               const std::string &replace, 
                               const std::string &subject, 
                               int32_t count = -1);

} //namespace string

} //namespace pf_basic

#endif //PF_BASIC_STRING_H_
