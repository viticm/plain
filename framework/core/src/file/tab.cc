#include "plain/file/tab.h"
#include <unordered_map>
#include "plain/basic/utility.h"
#include "plain/sys/assert.h"

using namespace plain;

Tab::Tab(uint32_t id) : 
  id_{id},
  record_number_{0},
  field_number_{0},
  string_buffer_{nullptr},
  string_buffer_size_{0},
  index_column_{INDEX_INVALID} {
}

Tab::~Tab() {
  if (!is_null(string_buffer_)) {
    delete[] string_buffer_;
    string_buffer_ = nullptr;
  }
}

bool Tab::open_from_txt(const char* filename) {
  Assert(filename);
  FILE *fp = fopen(filename, "rb");
  if (nullptr == fp) return false;
  fseek(fp, 0, SEEK_END);
  int32_t filesize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  //read in memory
  char* memory = new char[filesize + 1];
  memset(memory, 0, filesize + 1); //use memset to memory pointer
  fread(memory, 1, filesize, fp);
  fclose(fp);
  memory[filesize] = '\0';
  bool result = open_from_memory(memory, memory + filesize + 1, filename);
  safe_delete_array(memory);
  return result;
}

bool Tab::open_from_memory(const char* memory, 
                                const char* end, 
                                const char* filename) {
  bool result = true;
  if (end - memory >= static_cast<int32_t>(sizeof(file_head_t)) && 
      *((uint32_t*)memory) == FILE_DATABASE_INDENTIFY) {
    result = open_from_memory_binary(memory, end, filename);
  } else {
    result = open_from_memory_text(memory, end, filename);
  }
  return result;
}

const Tab::field_data* Tab::search_index_equal(int32_t index) const {
  field_hashmap::const_iterator it_find = hash_index_.find(index);
  if (it_find == hash_index_.end()) return nullptr;
  return it_find->second;
}

const char* Tab::get_fieldname(int32_t index) {
  const char* name = nullptr;
  Assert(index >= 0 && index <= field_number_);
  name = fieldnames_[index].c_str();
  return name;
}

int32_t Tab::get_fieldindex(const char* name) {
  int32_t result = -1;
  uint32_t i;
  for (i = 0; i < fieldnames_.size(); ++i) {
    if (0 == strcmp(name, fieldnames_[i].c_str())) {
      result = i;
      break;
    }
  }
  return result;
}

uint8_t Tab::get_fieldtype(int32_t index) {
  Assert(index >= 0 && index <= field_number_);
  uint8_t result = static_cast<uint8_t>(type_[index]);
  return result;
}

const Tab::field_data *Tab::search_position(int32_t line, 
                                            int32_t column) const {
  int32_t position = line * get_field_number() + column;
  if (line < 0 || position > static_cast<int32_t>(data_buffer_.size())) {
    char temp[256];
    memset(temp, '\0', sizeof(temp));
    snprintf(temp, 
             sizeof(temp) - 1, 
             "plain_file::Tab::search_position is failed,"
             " position out for range[line:%d, column:%d] position:%d",
             line,
             column,
             position);
#ifdef _plain_THROW_EXCEPTION_AS_STD_STRING
    throw std::string(temp);
#else
    AssertEx(false, temp);
#endif
    return nullptr;
  }
  return &(data_buffer_[position]);
}

const Tab::field_data* Tab::search_first_column_equal(
    int32_t column, 
    const field_data &value) const {
  if (column < 0 || column > field_number_) return nullptr;
  field_type_enum type = type_[column];
  register int32_t i;
  for (i = 0; i < record_number_; ++i) {
    const field_data &_field_data = 
      data_buffer_[(field_number_ * i) + column];
    bool result;
    if (kTypeInt == type) {
      result = field_equal(kTypeInt, _field_data, value);
    } else if (kTypeFloat == type) {
      result = field_equal(kTypeFloat, _field_data, value);
    } else {
      result = field_equal(kTypeString, _field_data, value);
    }
    if (result) {
      return &(data_buffer_[field_number_ * i]);
    }
  }
  return nullptr;
}

uint32_t Tab::get_id() const {
  return id_;
}

int32_t Tab::get_field_number() const {
  return field_number_;
}

int32_t Tab::get_record_number() const {
  return record_number_;
}

void Tab::create_index(int32_t column, const char* filename) {
  if (column < 0 || column > field_number_ || index_column_ == column) return;
  hash_index_.clear();
  int32_t i;
  for (i = 0; i < record_number_; ++i) {
    field_data* _field_data = &(data_buffer_[i * field_number_]);
    field_hashmap::iterator it_find = hash_index_.find(_field_data->int_value);
    if (it_find != hash_index_.end()) {
      char temp[256];
      memset(temp, '\0', sizeof(temp));
      snprintf(temp, 
               sizeof(temp) - 1, 
               "[%s]multi index at line: %d(smae value: %d)", 
               filename, 
               i + 1, 
               _field_data->int_value);
#ifdef _plain_THROW_EXCEPTION_AS_STD_STRING
      throw std::string(temp);
#else
      AssertEx(false, temp);
#endif
    }
    hash_index_.insert(std::make_pair(_field_data->int_value, _field_data));
  }
}

const char* Tab::get_line_from_memory(char* str, 
                                           int32_t size, 
                                           const char* memory, 
                                           const char* end) {
  register const char* _memory = memory;
  if (_memory >= end || 0 == *_memory) return nullptr;
  while (_memory < end &&
         _memory - memory + 1 < size &&
         *_memory != 0 &&
         *_memory != '\n' &&
         *_memory != '\r') {
    *(str++) = *(_memory++);
  }
  *str = 0;
  while (_memory < end && 
         *_memory != 0 && 
         (*_memory == '\r' || *_memory == '\n')) ++_memory;
  return _memory;
}

bool Tab::field_equal(field_type_enum type, 
                      const field_data &a, 
                      const field_data &b) {
  bool result = false;
  if (kTypeInt == type) {
    result = a.int_value == b.int_value;
  } else if (kTypeFloat == type) {
    result = a.float_value == b.float_value;
  } else {
    try {
      result = 0 == strcmp(a.string_value, b.string_value);
    } catch(...) {
      //do nothing
    }
  }
  return result;
}

bool Tab::open_from_memory_text(const char* memory, 
                                const char* end, 
                                const char* filename) {
  char line[(1024 * 10) + 1]; //long string
  memset(line, '\0', sizeof(line));
  register const char* _memory = memory;
  _memory = get_line_from_memory(line, sizeof(line) - 1, _memory, end);
  if (!_memory) return false;
  std::vector<std::string> result;
  explode(line, result, "\t", true, true);
  if (result.empty()) return false;
  field_type _field_type;
  _field_type.resize(result.size());
  int32_t i;
  uint32_t result_size = static_cast<uint32_t>(result.size());
  for (i = 0; i < static_cast<int32_t>(result_size); ++i) {
    if ("INT" == result[i]) {
      _field_type[i] = kTypeInt;
    } else if("FLOAT" == result[i]) {
      _field_type[i] = kTypeFloat;
    } else if("STRING" == result[i]) {
      _field_type[i] = kTypeString;
    } else {
      return false;
    }
  }
  //init
  int32_t record_number = 0;
  int32_t field_number = static_cast<int32_t>(_field_type.size());
  std::vector<std::pair<std::string, int32_t> > string_buffer;
  std::unordered_map<std::string, int32_t> map_string_buffer;
  _memory = get_line_from_memory(line, sizeof(line) - 1, _memory, end);
  //第二行为列名（相当于数据库的字段名），应尽量使用英文
  explode(line, fieldnames_, "\t", true, true);
  if (!_memory) return false;
  int32_t string_buffer_size = 0;
  bool loop = true;
  do {
    //以行读取数据
    _memory = get_line_from_memory(line, sizeof(line) - 1, _memory, end);
    if (!_memory) break;
    if ('#' == line[0]) continue; //注释行
    explode(line, result, "\t", true, false);
    if (result.empty()) continue; //空行
    if (static_cast<int32_t>(result.size()) != field_number) { //列数不对
      int32_t left_number = 
        field_number - static_cast<int32_t>(result.size());
      for (i = 0; i < left_number; ++i) {
        result.push_back("");
      }
    }
    if (result[0].empty()) continue;
    for (i = 0; i < field_number; ++i) {
      field_data _field_data;
      switch(_field_type[i]) {
        case kTypeInt: {
          _field_data.int_value = atoi(result[i].c_str());
          data_buffer_.push_back(_field_data);
          break;
        }
        case kTypeFloat: {
          _field_data.float_value = static_cast<float>(atof(result[i].c_str()));
          data_buffer_.push_back(_field_data);
          break;
        }
        case kTypeString: {
#ifdef FILE_DATABASE_CONVERT_GBK_TO_UTF8
          const char* value = result[i].c_str();
          //convert charset
          //utf8 -> gbk 1.5multiple length
          int32_t convert_strlength = static_cast<int32_t>(strlen(value) * 2);
          char* convert_str = new char[convert_strlength];
          memset(convert_str, 0, convert_strlength);
          int32_t convert_result = 
            charset_convert("GBK",
                            "UTF-8",
                             convert_str,
                             convert_strlength,
                             value,
                             static_cast<int32_t>(strlen(value)));
          if (convert_result > 0) {
            value = convert_str;
            result[i] = convert_str;
          }
          if (!is_null(convert_str)) {
            delete[] convert_str;
            convert_str = nullptr;
          }
#endif
          auto it = map_string_buffer.find(result[i]);
          if (it == map_string_buffer.end()) {
            string_buffer.push_back(
                std::make_pair(result[i], string_buffer_size));
            map_string_buffer.insert(
                std::make_pair(result[i], 
                static_cast<int32_t>(string_buffer.size()) - 1));
            _field_data.int_value = string_buffer_size + 1;
            string_buffer_size += 
              static_cast<int32_t>(strlen(result[i].c_str())) + 1;
          } else {
            _field_data.int_value = string_buffer[it->second].second + 1;
          }
          data_buffer_.push_back(_field_data);
          break;
        }
        default: {
          return false;
        }
      }
    }
    ++record_number;
  } while (loop); 
  //database init
  record_number_ = record_number;
  field_number_ = field_number;
  string_buffer_size_ = string_buffer_size + 1;
  string_buffer_ = new char[string_buffer_size_];
  type_ = _field_type;
  //unsigned char blank = '\0';
  string_buffer_[0] = '\0';
  
  register char* temp = string_buffer_ + 1;
  for (i = 0; i < static_cast<int32_t>(string_buffer.size()); ++i) {
    memcpy(temp, 
           string_buffer[i].first.c_str(), 
           string_buffer[i].first.size());
    temp += string_buffer[i].first.size();
    *(temp++) = '\0';
  }

  //relocate string block
  register uint16_t m, n;
  for (m = 0; m < field_number; ++m) {
    if (type_[m] != kTypeString) continue;
    for (n = 0; n < record_number; ++n) {
      field_data &_field_data1 = data_buffer_[(n * field_number) + m];
      _field_data1.string_value = string_buffer_ + _field_data1.int_value;
    }
  }
  create_index(0, filename);
  return true;
}

bool Tab::open_from_memory_binary(const char* memory, 
                             const char* end, 
                             const char* filename) {
  register const char* _memory = memory;
  file_head_t file_head;
  memcpy(&file_head, _memory, sizeof(file_head_t));
  if (file_head.identify != FILE_DATABASE_INDENTIFY) return false;
  //check memory size
  if (sizeof(file_head) +
      sizeof(uint32_t) * file_head.field_number +
      sizeof(field_data) * file_head.record_number * file_head.field_number +
      + file_head.string_block_size > static_cast<uint64_t>(end - memory)) {
    return false;
  }
  _memory += sizeof(file_head);
  
  //init 
  record_number_ = file_head.record_number;
  field_number_ = file_head.field_number;
  string_buffer_size_= file_head.string_block_size;
  
  //create string blok
  string_buffer_ = new char[string_buffer_size_];
  if (!string_buffer_) return false;
  std::vector<uint32_t> field_types;
  field_types.resize(field_number_);
  memcpy(&(field_types[0]), _memory, sizeof(uint32_t) * field_number_);

  //check it
  type_.resize(field_number_);
  int32_t i;
  for (i = 0; i < field_number_; ++i) {
    switch(field_types[i]) {
      case kTypeInt: {
        //do nothing
      }
      case kTypeFloat: {
        //do nothing
      }
      case kTypeString: {
        type_[i] = static_cast<field_type_enum>(field_types[i]);
        break;
      }
      default: {
        if (!is_null(string_buffer_)) {
          delete[] string_buffer_;
          string_buffer_ = nullptr;
        }
        return false;
      }
    }
  }

  //read all field
  data_buffer_.resize(field_number_ * record_number_);
  memcpy(&(data_buffer_[0]), 
         _memory, 
         sizeof(field_data) * field_number_ * record_number_);
  _memory += sizeof(field_data) * field_number_ * record_number_;
  memcpy(string_buffer_, _memory, string_buffer_size_);
  string_buffer_[string_buffer_size_ - 1] = '\0';

  //runtime address
  for (i = 0; i < field_number_; ++i) {
    if (field_types[i] != kTypeString) continue;
    std::string str;
    int32_t j;
    for (j = 0; j < record_number_; ++j) {
      data_buffer_[i + field_number_ + j].string_value += 
        reinterpret_cast<uint64_t>(string_buffer_); 
    }
  }
  create_index(0, filename);
  return true;
}

bool Tab::save_tobinary(const char* filename) {
    file_head_t filehead;
    filehead.field_number = field_number_;
    filehead.record_number = record_number_;
    filehead.string_block_size = string_buffer_size_;
    FILE *fp = fopen(filename, "wb");
    if (nullptr == fp) return false;
    fwrite(&filehead, sizeof(filehead), 1, fp);
    fwrite(&(type_[0]), sizeof(field_type_enum) *filehead.field_number, 1, fp);
    fwrite(&(data_buffer_[0]), 
           filehead.field_number * filehead.record_number, 
           1, 
           fp);
    fwrite(string_buffer_, filehead.string_block_size, 1, fp);
    fclose(fp);
    return true;
}

bool Tab::save_totext(const char* filename) {
  FILE *fp = fopen(filename, "wb");
  if (nullptr == fp) return false;

  for(int i=0; i< static_cast<int>(type_.size()); ++i){
    if(type_[i] == kTypeInt){
      char temp[64] = {0};
      snprintf(temp, sizeof(temp) - 1, "INT");
      fwrite(temp, strlen(temp), 1, fp); 
    } else if(type_[i] == kTypeFloat){
      char temp[64] = {0};
      snprintf(temp, sizeof(temp) - 1, "FLOAT");
      fwrite(temp, strlen(temp), 1, fp); 
    } else if(type_[i] == kTypeString){
      char temp[64] = {0};
      snprintf(temp, sizeof(temp) - 1, "STRING");
      fwrite(temp, strlen(temp), 1, fp); 
    }
    if(i == static_cast<int>(type_.size()-1)){
      fwrite("\n", 1, 1, fp);
    } else{
      fwrite("\t", 1, 1, fp);
    }
  }

  for(int i=0; i<static_cast<int>(fieldnames_.size()); ++i){
    char temp[64] = {0};
    snprintf(temp, sizeof(temp) - 1, fieldnames_[i].c_str());
    fwrite(temp, strlen(temp), 1, fp); 
    if(i == static_cast<int>(fieldnames_.size()-1)){
      fwrite("\n", 1, 1, fp);
    } else{
      fwrite("\t", 1, 1, fp);
    } 
  }

  for (int32_t line = 0; line < record_number_; ++line) {
    for (int32_t column = 0; column < field_number_; ++column) {
      const field_data *_field_data = search_position(line, column);
      switch (type_[column]) {
        case kTypeInt: {
          char temp[64] = {0};
          snprintf(temp, sizeof(temp) - 1, "%d", _field_data->int_value);
          fwrite(temp, strlen(temp), 1, fp);
          break;
        }
        case kTypeFloat: {
          char temp[64] = {0};
          snprintf(temp, sizeof(temp) - 1, "%.3f", _field_data->float_value);
          fwrite(temp, strlen(temp), 1, fp);
          break;
        }
        case kTypeString: {
          fwrite(_field_data->string_value, 
                 strlen(_field_data->string_value), 
                 1, 
                 fp);
          break;
        }
        default:
          break;
      }

      fwrite("\t", 1, 1, fp);
    } //for
    fwrite(LF, strlen(LF), 1, fp);
  } //for
  fclose(fp);
  return true;
}

const Tab::field_data *Tab::get_fielddata(int32_t line, 
                                          const char* name) {
  const field_data *_field_data = nullptr;
  int32_t column = get_fieldindex(name);
  _field_data = search_position(line, column);
  return _field_data;
}

bool Tab::save_totext_line(const std::vector<std::string> &_data){
  if(static_cast<int32_t>(_data.size()) != field_number_)
    return false;

  for(int32_t column = 0; column < field_number_; ++column){
    switch (type_[column]) {
      case kTypeInt: {
      
        break;
      }
      case kTypeFloat: {
      
        break;
      }
      case kTypeString: {
      
        break;
      }
      default:
        break;

    }
  } 
  return true;
}
