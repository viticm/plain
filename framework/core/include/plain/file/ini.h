/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id ini.h
 * @link https://github.com/viticm/plain1 for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm@126.com/viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm@126.com/viticm.ti@gmail.com>
 * @date 2023/04/01 21:05
 * @uses file ini class
 */
#ifndef PLAIN_FILE_INI_H_
#define PLAIN_FILE_INI_H_

#include "plain/file/config.h"
#include <unordered_map>
#include "plain/basic/type/config.h"

#define INI_VALUE_MAX 1024
#define FILE_INI_NOTE (';')

namespace plain {

class PLAIN_API Ini {

 public:
   Ini();
   Ini(const char *filename);
   ~Ini();

 public:
   typedef std::unordered_map<std::string, int32_t> valueset_t;
   typedef std::unordered_map<std::string, valueset_t *> sectionset_t;

 public:
   bool open(const char *filename);
   void close();

 public:

   //Get variable.
   void get(const char *section, 
            const char *key, 
            plain::variable_t &variable);

   //Get data.
   sectionset_t *getdata();

   //Read string by position;
   const char *getstring(int32_t position);

 public:

   int32_t getint32(const char *section, const char *key, int32_t _default = 0);
   bool getint32_ifexist(const char *section, const char *key, int32_t &result);
   float getfloat(const char *section, const char *key, float _default = 0.0f);
   bool getfloat_ifexist(const char *section, const char *key, float &result);
   const char *getstring(const char *section, const char *key);
   bool getstring(const char *section, 
                  const char *key, 
                  char *str, 
                  int32_t size, 
                  const char *_default = "");
   bool getstring_ifexist(const char *section, 
                          const char *key, 
                          char *str, 
                          int32_t size);

 private:
   bool _getint32(const char *section, const char *key, int32_t &result);
   bool _getfloat(const char *section, const char *key, float &result);
   bool _getstring(const char *section, 
                   const char *key, 
                   char *str, 
                   int32_t size);
   void _trimstring(char *buffer);
   bool _parsesection(char *buffer, char **section);
   bool _parsekey(char *buffer, char **key, char **value);
   void _buffer_resize();
   char *get_bufferstring(int32_t position);
   int32_t _add_bufferstring(char *str);

 private:
   std::ifstream fstream_;
   char filename_[FILENAME_MAX];
   sectionset_t sectiondata_;
   valueset_t *current_section_{nullptr};
   char *buffer_{nullptr};
   int32_t bufferlength_{0};
   int32_t bufferlength_max_{0};

};

} //namespace plain

#endif //PLAIN_FILE_INI_H_
