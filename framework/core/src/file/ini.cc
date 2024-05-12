#include "plain/file/ini.h"
#include "plain/basic/utility.h"
#include "plain/basic/type/variable.h"
#include "plain/sys/assert.h"

using namespace plain;

Ini::Ini() noexcept :
  current_section_{nullptr},
  buffer_{nullptr},
  bufferlength_{0},
  bufferlength_max_{0} {
  memset(filename_, 0, FILENAME_MAX);
}

Ini::Ini(const char *filename) noexcept :
  current_section_{nullptr},
  buffer_{nullptr},
  bufferlength_{0},
  bufferlength_max_{0} {
  memset(filename_, 0, FILENAME_MAX);
  open(filename);
}

Ini::~Ini() {
  close();
}

bool Ini::open(const char *filename) noexcept {
  buffer_ = new char[INI_VALUE_MAX];
  bufferlength_max_ += INI_VALUE_MAX; 
  strncpy(filename_, filename, FILENAME_MAX - 1);
  fstream_.open(filename, std::ios_base::in);
  if (!fstream_) {
    char msg[128] = {0};
    snprintf(msg, 128, "[file] open file failed! file: %s", filename);
    AssertEx(false, msg);
    return false;
  }
  char buffer[512] = {0};
  while (fstream_.getline(buffer, 512)) {
    _trimstring(buffer);
    char *section;
    if(_parsesection(buffer, &section)) {
      current_section_ = new valueset_t;
      sectiondata_.insert(
          std::make_pair(std::string(section), current_section_));
      continue;
    }
    char *key = nullptr, *value = nullptr;
    if (_parsekey(buffer, &key, &value)) {
      if(current_section_ == nullptr) {
        char msg[128] = {0};
        snprintf(msg, 
                 128, 
                 "[file] format invalid! file: %s, key: %s", 
                 filename,
                 key);
        AssertEx(false, msg);
      }
      int position = _add_bufferstring(value);
      current_section_->insert(std::make_pair(std::string(key), position));
      continue;
    }
  }
  return true;
}

void Ini::close() noexcept {
  if (fstream_) fstream_.close();
  safe_delete_array(buffer_);
  sectionset_t::iterator iter = sectiondata_.begin();
  for (; iter != sectiondata_.end(); iter++) {
    valueset_t *_section = iter->second;
    safe_delete(_section);
  }
  sectiondata_.clear();
  bufferlength_ = 0;
  bufferlength_max_ = 0;
  memset(filename_, 0, FILENAME_MAX);
}

Ini::sectionset_t *Ini::getdata() noexcept {
  return &sectiondata_;
}

int32_t Ini::getint32(const char *section, const char *key, int32_t _default) {
  int32_t result;
  if (_get(section, key, result)) return result;
  char msg[5120] = {0};
  snprintf(msg, 
           sizeof(msg) - 1, 
           "[ini] invalid key! file: %s, section: %s, key: %s", 
           filename_,section,key);
  AssertEx(false, msg);
  return _default;
}

bool Ini::getint32_ifexist(const char *section, 
                           const char *key, 
                           int32_t &result) {
  return _get(section, key, result);
}

float Ini::getfloat(const char *section, const char *key, float _default) {
  float result;
  if (_get(section, key, result)) return result;  
  char msg[5120] = {0};
  snprintf(msg, 
           sizeof(msg) - 1, 
           "[ini] invalid key! file: %s, section: %s, key: %s", 
           filename_,
           section,
           key);
  AssertEx(false, msg);
  return _default;
}

bool Ini::getfloat_ifexist(const char *section, 
                           const char *key, 
                           float &result) {
  return _get(section, key, result);
}

bool Ini::getstring(const char *section, 
                    const char *key, 
                    char *str, 
                    int32_t size, 
                    const char *_default) {
  if (_getstring(section, key, str, size)) return true;
  strncpy(str, _default, size);
  int32_t _size = static_cast<int32_t>(strlen(_default)) > size - 1 ? 
                  size - 1 : 
                  static_cast<int32_t>(strlen(_default));
  str[_size] = 0;
  char msg[5120] = {0};
  snprintf(msg, 
           sizeof(msg) - 1, 
           "[ini] invalid key! file: %s, section: %s, key: %s", 
           filename_,section,key);
  AssertEx(false, msg);
  return false;
}

void Ini::get(
  const char *section, const char *key, plain::variable_t &variable) noexcept {
  sectionset_t::iterator it = sectiondata_.find(section);
  if(it == sectiondata_.end()) return;
  valueset_t *_section = it->second;
  if (_section != nullptr) {
    valueset_t::iterator it2 = _section->find(std::string(key));
    if (it2 == _section->end()) return;
    int position = it2->second;
    const char *value = get_bufferstring(position);
    variable = value;
  }
}

bool Ini::getstring_ifexist(const char *section, 
                            const char *key, 
                            char *str, 
                            int32_t size) {
  return _getstring(section, key, str, size);
}

bool Ini::_get(const char *section, const char *key, int32_t &result) {
  sectionset_t::iterator it = sectiondata_.find(section);
  if (it == sectiondata_.end()) return false;
  valueset_t *_section = it->second;
  if (_section != nullptr) {
    valueset_t::iterator it2 = _section->find(std::string(key));
    if(it2 == _section->end())
      return false;
    int position = it2->second;
    char *value = get_bufferstring(position);
    result = atoi(value);
    return true;
  }
  return false;
}

bool Ini::_get(const char *section, const char *key, float &result) {
  sectionset_t::iterator it = sectiondata_.find(section);
  if (it == sectiondata_.end()) return false;
  valueset_t *_section = it->second;
  if (_section != nullptr) {
    valueset_t::iterator it2 = _section->find(std::string(key));
    if (it2 == _section->end()) return false;
    int position = it2->second;
    char *value = get_bufferstring(position);
    result = (float)atof(value);
    return true;
  }
  return false;
}
 
const char *Ini::getstring(int32_t position) noexcept {
  return get_bufferstring(position);
}

bool Ini::_getstring(
  const char *section, const char *key, char *str, int32_t size) noexcept {
  sectionset_t::iterator it = sectiondata_.find(section);
  if(it == sectiondata_.end()) return false;
  valueset_t *_section = it->second;
  if (_section != nullptr) {
    valueset_t::iterator it2 = _section->find(std::string(key));
    if (it2 == _section->end()) return false;
    int position = it2->second;
    char *value = get_bufferstring(position);
    size_t valuelength = strlen(value);
    safecopy(str, value, valuelength + 1);
    size_t _size = 
      valuelength > static_cast<size_t>(size - 1) ? size - 1 : valuelength;
    str[_size] = 0;
    return true;
  }
  return false;
}

void Ini::_trimstring(char *buffer) {
  auto size = strlen(buffer);
  if (0 == size) return;
  if ('\r' == buffer[size - 1] || 
      '\n' == buffer[size - 1]) {
    buffer[strlen(buffer)-1] = 0;
  }
  size_t i;
  for (i = 0; i < strlen(buffer); ++i ){
    if(buffer[i] == FILE_INI_NOTE) {
      buffer[i] = '\0';
      break;
    }
  }
  for (i = strlen(buffer) - 1; i > 0; --i) {
    if (' '== buffer[i] || '\t' == buffer[i]) {
      buffer[i] = 0;
    } else {
      break;
    }
  }
}

bool Ini::_parsesection(char *buffer, char **_section) {
  if (buffer[0] != '[') return false;
  char *size = strchr(buffer, ']');
  if (size != nullptr) {
    size[0] = 0;
    *_section = buffer + 1;
    return true;
  }
  return false;
}

bool Ini::_parsekey(char *buffer, char **key, char **value) {
  char *size = strchr(buffer, '=');
  if (size != nullptr) {
    size[0] = 0;
    *key = buffer;
    *value = size + 1;
    return true;
  }
  return false;
}

void Ini::_buffer_resize() {
  bufferlength_max_ *= 2;
  buffer_ = (char*)realloc(buffer_, bufferlength_max_);
  if (is_null(buffer_)) Assert(false);
}

char *Ini::get_bufferstring(int32_t position) {
  if (position >= bufferlength_) return nullptr;
  return buffer_ + position;
}

int32_t Ini::_add_bufferstring(char *str) {
  int32_t position = 0;
  size_t size = strlen(str);
  if (bufferlength_ + static_cast<int32_t>(size + 1) >= bufferlength_max_) 
    _buffer_resize();
  memcpy(buffer_ + bufferlength_, str, size + 1);
  position = bufferlength_;
  bufferlength_ += static_cast<int32_t>(size) + 1;
  return position;
}
