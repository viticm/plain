#include <limits>
#ifdef PF_OPEN_ICONV
#include <iconv.h>
#endif //PF_OPEN_ICONV
#include "pf/basic/base64.h"
#include "pf/sys/assert.h"
#include "pf/basic/io.tcc"
#include "pf/basic/time_manager.h"
#include "pf/basic/string.h"

namespace pf_basic {

namespace string {

/***
  * 快速字符串转换成整数模板通用函数
  * @str: 需要被转换的字符串
  * @result: 存储转换后的结果
  * @max_length: 该整数类型对应的字符串的最多字符个数，不包括结尾符
  * @converted_length: 需要转换的字符串长度，如果为0则表示转换整个字符串
  * @ignored_zero: 是否忽略开头的0
  * @return: 如果转换成功返回true, 否则返回false
  */
template <typename IntType>
static bool fast_toint(const char *str,
             IntType &result, 
             uint8_t max_length, 
             uint8_t converted_length, 
             bool ignored_zero) {
  bool negative = false;
  const char *tmp_str = str;
  if (nullptr == str) return false;

  // 处理负数
  if ('-' == tmp_str[0]) {
    // 负数
    negative = true;
    ++tmp_str;
  }

  // 处理空字符串
  if ('\0' == tmp_str[0]) return false;

  // 处理0打头的
  if ('0' == tmp_str[0]) {
    // 如果是0开头，则只能有一位数字
    if (('\0' == tmp_str[1]) || (1 == converted_length)) {
      result = 0;
      return true;
    } else {
      if (!ignored_zero) return false;
      for (;;) {
        ++tmp_str;
        if (tmp_str - str > max_length-1) return false;
        if (*tmp_str != '0') break;
      }
      if ('\0' == *tmp_str) {
        result = 0;
        return true;
      }
    }
  }

  // 检查第一个字符
  if ((*tmp_str < '0') || (*tmp_str > '9')) return false;
  result = (*tmp_str - '0');

  while ((0 == converted_length) || (tmp_str - str < converted_length-1)) {
    ++tmp_str;
    if ('\0' == *tmp_str) break;
    if (tmp_str - str > max_length-1) return false;

    if ((*tmp_str < '0') || (*tmp_str > '9')) return false;

    result = result * 10;
    result += (*tmp_str - '0');
  }

  if (negative) result = -result;
  return true;
}

void replace_all(std::string& str, 
         const std::string source, 
         const std::string destination) {
  size_t position = str.find(source, 0);
  while (position != std::string::npos) {
    str.replace(position, source.length(), destination);
    position = str.find(source, position);
  }
}

bool toint32(const char *source, 
       int32_t &result, 
       uint8_t converted_length, 
       bool ignored_zero) {
  if (nullptr == source) return false;

  long value;
  if (!fast_toint<long>(
      source, 
      value, 
      sizeof("-2147483648") - 1, 
      converted_length, 
      ignored_zero)) 
    return false;
#undef max
#undef min
  auto _max = std::numeric_limits<int32_t>::max();
  auto _min = std::numeric_limits<int32_t>::min();
  if (value < _min || value > _max)  return false;
  result = static_cast<int32_t>(value);
  return true;
}

bool toint16(const char *source, 
       int16_t &result, 
       uint8_t converted_length, 
       bool ignored_zero) {
  int32_t value = 0;
  if (!toint32(source, value, converted_length, ignored_zero)) 
    return false;
  if (value < std::numeric_limits<int16_t>::min() ||
    value > std::numeric_limits<int16_t>::max()) return false;
  result = static_cast<int16_t>(value);
  return true;
}

char get_base64char(int index) {
  const char str[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi"
           "jklmnopqrstuvwxyz0123456789+/";
  if ((index >= 0) && (index < 64)) {
    return str[index];
  }
  return '=';
}

void encrypt(const char *in, char *out, int32_t out_length) {
  int32_t insize = static_cast<int>(strlen(in));
  if (insize <= 0) return;
  int32_t middle = 0 == insize % 2 ? insize / 2 : (insize + 1) / 2;
  int32_t length = insize + 2 + 3 + 1;
  auto src = new char[length];
  auto temp = new unsigned char[length + length / 3 + 10]; //enough output size
  memset(src, 0, length);
  memset(temp, 0, length);
  int32_t i, j, index;
  std::default_random_engine rand_engine(static_cast<uint32_t>(time(nullptr)));
  std::uniform_int_distribution<int> dis(0, 100);
  auto rand_gen = std::bind(dis, rand_engine);
  i = j = 0;
  for (; i < length; ++i) {
    index = rand_gen();
    if (i < 2 || (middle <= i && middle + 3 > i) || i == length - 1) {
      src[i] = get_base64char(index);
      continue;
    }
    src[i] = in[j++];
  }
  std::string encode_str = base64_encode(temp, length);
  strncpy(out, encode_str.c_str(), out_length);
  out[out_length - 1] = '\0';
  safe_delete_array(src);
  safe_delete_array(temp);
}

void decrypt(const char *in, char *out, int32_t out_length) {
  int32_t insize = static_cast<int>(strlen(in));
  if (insize <= 0) return;
  char *temp = new char[insize - insize / 3 + 10]; //enough buffer size.
  std::string str = base64_decode(in);
  int32_t length = static_cast<int32_t>(strlen(temp));
  int32_t right_length = length - 2 - 3 - 1;
  char *_temp = new char[right_length + 1];
  int32_t middle = //用正确的长度算出中间值
    0 == right_length % 2 ? right_length / 2 : (right_length + 1) / 2;
  int i, j;
  i = j = 0;
  for (; i < length; ++i) {
    if (i < 2 || (middle <= i && middle + 3 > i) || i == length - 1) {
      continue;
    }
    _temp[j++] = temp[i];
  }
  strncpy(out, temp, out_length);
  out[out_length - 1] = '\0';
  safe_delete_array(temp);
  safe_delete_array(_temp);
}

uint32_t crc(const char *str) {
  if (nullptr == str|| 0 == str[0]) return 0;
  uint32_t crc32 = 0xFFFFFFFF;
  int32_t size = static_cast<int32_t>(strlen(str));
  uint16_t i;
  for (i = 0; i < size; ++i) {
    crc32 = crc32 * 33 + static_cast<unsigned char>(str[i]);
  }
  return crc32;
  return 0;
}

char *safecopy(char *dest, const char *src, size_t size) {
  Assert(dest && src);
  strncpy(dest, src, size);
  dest[size - 1] = '\0';
  return dest;
}

/**
 * @desc this function can convert charset with diffrent
 * @param from source charset(example: utf8)
 * @param to destination charset
 * @param save want save string
 * @param savelen want save string length
 * @param src want convert string
 * @param srclen want convert string length
 */
#ifdef PF_OPEN_ICONV
int32_t charset_convert(const char *from, 
            const char *to, 
            char *save, 
            int32_t save_length, 
            const char *src, 
            int32_t src_length) {
  int32_t status = 0;
  iconv_t cd;
  const char *inbuf  = src;
  char *outbuf     = save;
  size_t outbufsize  = save_length;
  size_t savesize  = 0;
  size_t inbufsize   = src_length;
  const char *inptr  = inbuf;
  size_t insize    = inbufsize;
  char *outptr     = outbuf;
  size_t outsize   = outbufsize;

  cd = iconv_open(to, from);
  iconv(cd, nullptr, nullptr, nullptr, nullptr);
  if (0 == inbufsize) {
    status = -1;
    goto done;
  }
  while (0 < insize) {
    auto res = iconv(cd, (const char **)&inptr, &insize, &outptr, &outsize);
    if (outptr != outbuf) {
      std::unique_ptr<char []> _outbuf(new char[outsize + 1]);
      memset(_outbuf.get(), 0, outsize + 1);
      strncpy(_outbuf.get(), outbuf, outsize);
      int32_t saved_errno = errno;
      outsize = static_cast<int32_t>(outptr - outbuf);
      strncpy(save + savesize, _outbuf.get(), outsize);
      errno = saved_errno;
    }
    if ((size_t)(-1) == res) {
      if (EILSEQ == errno) {
        int one = 1 ;
        iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &one);
        status = -3;
      } else if (EINVAL == errno) {
        if (0 == inbufsize) {
          status = -4;
          goto done;
        } 
        else {
          break;
        }
      } else if (E2BIG == errno) {
        status = -5;
        goto done;
      } else {
        status = -6;
        goto done;
      }
    }
  }
  status = static_cast<int32_t>(strlen(save));
  done:
  iconv_close(cd);
  return status;
}
#else
int32_t charset_convert(
    const char *, const char *, char *, int32_t, const char *, int32_t) {
#if _DEBUG
  io_cwarn("pf_basic::string::charset_convert not work without iconv.");
#endif
  return 0;
}
#endif //PF_OPEN_ICONV

bool get_escapechar(char in, char& out) {
  const char char_array[][2] = {
    {0, '0'},
    {'\n', 'n'},
    {'\r', 'r'},
    {0x1a, 'Z'},
    {'\"', '\"'},
    {'\'', '\''},
    {'\\', '\\'}
  };
  for (uint8_t i = 0; i < sizeof(char_array)/sizeof(char_array[0]); ++i) {
    if (char_array[i][0] == in) {
    out = char_array[i][1];
    return true;
    }
  }
  return false;
}

bool getescape(const char *in, size_t insize, char *out, size_t outsize) {
  size_t index1 = 0, index2 = 0;
  for ( ; index1 < insize && index2 < outsize; ++index1) {
    char _char;
    if (get_escapechar(in[index1], _char)) {
    out[index2] = '\\';
    ++index2;
    out[index2] = _char;
    ++index2;
    } else {
    out[index2] = in[index1];
    ++index2;
    }
  }
  return true;
}

int64_t toint64(const char *str) {
  char *endpointer = nullptr;
  int64_t result = strtoint64(str, &endpointer, 10);
  return result;
}

uint64_t touint64(const char *str) {
  char *endpointer = nullptr;
  int64_t result = strtouint64(str, &endpointer, 10);
  return result;
}

int32_t explode(const char *source,
        std::vector<std::string> &result,
        const char *key,
        bool one_key,
        bool ignore_empty) {
  result.clear();
  std::string str = source; //use stanard string class to source
  if (str.empty()) return 0;
  std::string::size_type left = 0;
  std::string::size_type right;
  if (one_key) {
    right = str.find_first_of(key);
  } else {
    right = str.find(key);
  }

  if (std::string::npos == right) right = str.length();
  for(;;) {
    std::string item = str.substr(left, right - left);

    if (item.length() > 0 || !ignore_empty) result.push_back(item);
    if (right == str.length()) break;
    left = right + (one_key ? 1 : strlen(key));
    if (one_key) {
      std::string temp = str.substr(left);
      right = temp.find_first_of(key);
      if (right != std::string::npos) right += left;
    } else {
      right = str.find(key, left);
    }
    if (std::string::npos == right) right = str.length();
  }
  int32_t _result = static_cast<int32_t>(result.size());
  return _result;
}

bool checkstr(const char *in, uint32_t size) {
  if (0 == size) return false;
  for (decltype(size) i = 0; i < size; ++i) {
    switch (in[i]) {
    case '\0':
      return true;
    case '\'':
      return false;
    case '\"':
      return false;
    case '(':
      return false;
    case ')':
      return false;
    case '=':
      return false;
    default:
      break;
    }
  }
  return true;
}

//A simple de/encrypt for password.

//public use
#define PG_MAXBUFFER 64
#define PG_MAXPASSWORDLEN  20
#define PG_RESULTLENSTD  32
//private use
#define PG_MINPKEYLEN (PG_RESULTLENSTD - PG_MAXPASSWORDLEN - 2)
#define PG_MAXPKEYLEN (PG_RESULTLENSTD - 2)
#define PG_PKEYMASK 151
#define PG_PKEYOFF 7
#define PG_ENOFF 5
#define PG_MINCHAR 0x20
#define PG_MAXCHAR 0x7E
#define PG_CHARCOUNT ((PG_MAXCHAR - PG_MINCHAR + 1))
#define PG_VALIDCHAR(c) ((c >= 'a' && c <= 'z') || \
    (c >= 'A' && c <= 'Z') || \
    (c == '_') || \
    (c >= '0' && c <= '9'))
#define PG_INVALIDCHAR(c) !PG_VALIDCHAR(c)

inline int32_t pg_char2int(char b) {
  int32_t x{0}, y{0};
  int32_t keyoff = PG_PKEYOFF % 8;
  x = b ^ PG_PKEYMASK;
  y = 0x1 & x;
  y = y << keyoff;
  x = x >> (8 - keyoff);
  x = x | y;
  x = x & 0x1f;
  return x;
}

static char int2char(char i, uint64_t tickcount = 0) {
  char b;
  Assert(i >= 0 && i <= PG_RESULTLENSTD - 2);
  int size{0};
  int index{0};
  for (index = tickcount % PG_CHARCOUNT, 
       size = index + PG_CHARCOUNT; 
       index < size; index++) {
    b = index % PG_CHARCOUNT + PG_MINCHAR;
    if (pg_char2int(b) == i && PG_VALIDCHAR(b))
      return b;
  }
  for (index = tickcount % PG_CHARCOUNT, 
       size = index + PG_CHARCOUNT; 
       index < size; index++) {
    b = index % PG_CHARCOUNT + PG_MINCHAR;
    if (pg_char2int(b) == i)
      return b;
  }
  Assert(0);
  return (char)0xFF;
}

inline void pg_swapchars(char *sz) {
  char c{0};
#define PG_SWAP(n1, n2) c = sz[n1]; sz[n1] = sz[n2]; sz[n2] = c
  PG_SWAP(0, 13);
  PG_SWAP(31, 25);
  PG_SWAP(12, 30);
  PG_SWAP(7, 19);
  PG_SWAP(3, 21);
  PG_SWAP(9, 20);
  PG_SWAP(15, 18);
}

static bool pg_encrypt(char *key, 
                       int32_t keylength, 
                       char *buffer, 
                       const char *password, 
                       int32_t str_length) {
  int32_t i{0}, c{0}, cc{0};
  for (i = 0; i < str_length; i++) {
    cc = key[i % keylength];
    c = password[i];
    c = (((c - PG_MINCHAR ) + (cc - PG_MINCHAR)) % PG_CHARCOUNT) + PG_MINCHAR;
    if (PG_INVALIDCHAR(c)) {
      int32_t loop{0};
      do {
        c++;
        if (c > PG_MAXCHAR)
          c = PG_MINCHAR;
        cc++;
        if (cc > PG_MAXCHAR)
          cc = PG_MINCHAR;
        key[i % keylength] = (char)cc;
        if (loop++ > 255)
          return false;
      } while (PG_INVALIDCHAR(cc) || PG_INVALIDCHAR(c));
    }
    buffer[i] = (char)c;
  }
  return true;
}

static void pg_decrypt(char *key, 
                       int32_t keylength, 
                       char *buffer, 
                       const char *encrypted, 
                       int32_t length) {
  int i{0}, c{0}, cc{0};
  for (i = 0; i < length; i++) {
    cc = key[i % keylength];
    c = encrypted[i];
    buffer[i] = 
      ((PG_CHARCOUNT + (c - PG_MINCHAR ) - (cc - PG_MINCHAR)) % PG_CHARCOUNT) + 
      PG_MINCHAR;
  }
  buffer[i] = 0;
}

static bool pg_decrypt_password(char *password, const char *encrypted) {
  int32_t length = (int32_t)strlen(encrypted), pk_length{0}, i{0};
  char buffer[PG_RESULTLENSTD + 1]{0};
  if (length != PG_RESULTLENSTD)
    return false;

  for (i = 0; i < PG_RESULTLENSTD; i++)
    buffer[i] = encrypted[i];
  buffer[PG_RESULTLENSTD] = 0;

  pg_swapchars(buffer);

  pk_length = pg_char2int(buffer[0]);
  if (pk_length < PG_MINPKEYLEN || pk_length > PG_MAXPKEYLEN)
    return false;
  length = pg_char2int(buffer[pk_length + 1]);
  if (length < 0 || length > PG_MAXPASSWORDLEN)
    return false;
  pg_decrypt(buffer + 1, pk_length, password, buffer + pk_length + 2, length);

  return true;
}

static bool pg_encrypt_password(
    char *result, const char *password, uint64_t tickcount, int32_t &level) {
  ++level;
  if (level > 32) return false;

  int prime_number_list[PG_RESULTLENSTD] = {
    1153,  1789,  2797,  3023,  3491,  3617,  4519,  4547,
    5261,  5939,  6449,  7307,  8053,  9221,  9719,  9851,
    313,  659,  1229,  1847,  2459,  3121,  3793,  4483,
    5179,  6121,  6833,  7333,  7829,  8353,  9323,  9829,
  };

  int32_t length = (int32_t)strlen(password), keylength = 0;
  bool encryptok{false};
  if (length > PG_MAXPASSWORDLEN) return false;
  for (int32_t i = 0; i < length; ++i) {
    int32_t val{0}; val = static_cast<int32_t>(password[i]);
    if (val < PG_MINCHAR || val > PG_MAXCHAR) return false;
  }

  keylength = PG_RESULTLENSTD - length - 2;
  if (keylength > PG_MINPKEYLEN)
    keylength = 
      (tickcount + 10237) % (keylength - PG_MINPKEYLEN) + PG_MINPKEYLEN;

  result[0] = int2char((char)keylength, tickcount);

  //public key
  for (int32_t i = 0; i < keylength; i++) {
    uint32_t random = (uint32_t)tickcount + prime_number_list[i];
    char c = (char)((random % PG_CHARCOUNT) + PG_MINCHAR);
    if (PG_INVALIDCHAR(c))
      c = (char)((random & 1) ? 'a' + (random % 26) : 'A' + (random % 26));
    result[i + 1] = c;
  }

  result[keylength + 1] = int2char((char)length, tickcount);
  encryptok = 
    pg_encrypt(result + 1, keylength, result + keylength + 2, password, length);

  //fill
  for (int32_t i = 0; i < PG_RESULTLENSTD - 2 - keylength - length; i++) {
    uint32_t random = (uint32_t)(tickcount + prime_number_list[PG_RESULTLENSTD - i - 1]);
    char c = (char)((random % PG_CHARCOUNT) + PG_MINCHAR);
    if (PG_INVALIDCHAR(c))
      c = (char)((random & 1) ? 'a' + (random % 26) : 'A' + (random % 26));
    result[PG_RESULTLENSTD - i - 1] = c;
  }
  pg_swapchars(result);
  result[PG_RESULTLENSTD] = 0;

  char password_check[PG_RESULTLENSTD + 1]{0};
  bool decrypt_ok{false};

  //recheck password
  if (encryptok) decrypt_ok = pg_decrypt_password(password_check, result);
  if (!encryptok || !decrypt_ok || stricmp(password_check, password) != 0)
    return pg_encrypt_password(result, password, tickcount + 9929, level);
  return false;
}

bool encrypt(const std::string &in, std::string &out) {
  auto tickcount = TIME_MANAGER_POINTER->get_tickcount();
  char temp[PG_RESULTLENSTD + 1]{0};
  int level = 0;
  bool result{false};
  if (pg_encrypt_password(temp, in.c_str(), tickcount, level)) {
    result = true;
    out = result;
  }
  return result;
}

bool decrypt(const std::string &in, std::string &out) {
  char temp[PG_RESULTLENSTD + 1]{0};
  if (!pg_decrypt_password(temp, in.c_str())) return false;
  out = temp;
  return true;
}

bool encrypt_number(int32_t number, char _char, std::string &out) {
  std::string number_str = std::to_string(number);
  auto number_len = number_str.size();
  out = "";
  for (decltype(number_len) i = 0; i < number_len; ++i) {
    auto n = number / static_cast<int32_t>((pow(10, number_len - i - 1))) % 10;
    char c = (char)n + _char;
    out = out + c;
  }
  return true;
}

bool decrypt_number(const std::string &in, char _char, int32_t &number) {
  auto number_len = in.size();
  number = 0;
  for (decltype(number_len) i = 0; i < number_len; ++i) {
    number += (in[i] - _char) 
              * static_cast<int32_t>(pow(10, number_len - i -1));
  }
  return true;
}

//This encrypt not change any word in old string, just insert the number as rand
//string in rand position.
//The final string is: first_char + len + pos_len + pos_str + newstring.
bool encrypt(const std::string &in, int32_t number, std::string &out) {
  const char *chars{"abcdefghijklmABCDEFGHIJKLM"}; 
  std::default_random_engine rand_engine(static_cast<uint32_t>(time(nullptr)));
  std::uniform_int_distribution<int32_t> dis(0, 25);
  auto rand_gen = std::bind(dis, rand_engine); rand_gen();
  auto first_char = chars[rand_gen()];
  std::string number_str{""};
  encrypt_number(number, first_char, number_str);
  auto number_len = number_str.size();
  std::uniform_int_distribution<size_t> dis1(0, in.size());
  auto rand_pos = std::bind(dis1, rand_engine); rand_pos();
  auto pos = rand_pos();
  std::string pos_str{""};
  encrypt_number((int32_t)pos, first_char, pos_str);
  // char pos_char = first_char + pos;
  char len_char = first_char + (char)number_len; //The length is 0 - 9.
  char pos_len_char = first_char + (char)pos_str.size(); 
  out = "";
  out = out + first_char + len_char + pos_len_char + pos_str;
  if (0 == pos) {
    out = out + number_str + in;
  } else if (pos == in.size()) {
    out = out + in + number_str;
  } else {
    out = out + in.substr(0, pos) + number_str + in.substr(pos);
  }
  return true;
}

bool decrypt(const std::string &in, int32_t &number, std::string &out) {
  if (in.size() <= 4) return false;
  char first_char = in[0];
  int32_t length = in[1] - first_char;
  int32_t pos_len = in[2] - first_char;
  int32_t header_size = 3 + pos_len;
  if (in.size() < (size_t)header_size) return false;
  int32_t pos{0};
  auto pos_str = in.substr(3, pos_len);
  decrypt_number(pos_str, first_char, pos);
  size_t rsize = in.size() - header_size;
  if (rsize < (size_t)(pos + length)) return false;
  std::string number_str = in.substr(header_size + pos, length);
  decrypt_number(number_str, first_char, number);
  out = in.substr(header_size, pos) + 
        in.substr(header_size + pos + length);
  return true;
}

std::string &ltrim(std::string &str, const std::string &character_mask) {
  if (str.empty()) return str;
  for (size_t i = 0; i < character_mask.length(); ++i) {
    str.erase(0, str.find_first_not_of(character_mask[i]));
  }
  return str;
}

std::string &rtrim(std::string &str, const std::string &character_mask) {
  if (str.empty()) return str;
  for (size_t i = 0; i < character_mask.length(); ++i) {
    str.erase(str.find_last_not_of(character_mask[i]) + 1);
  }
  return str;
}

std::string &trim(std::string &str, const std::string &character_mask) {
  if (str.empty()) return str;
  for (size_t i = 0; i < character_mask.length(); ++i) {
    str.erase(0, str.find_first_not_of(character_mask[i]));
    str.erase(str.find_last_not_of(character_mask[i]) + 1);
  }
  return str;
}

bool contains(
    const std::string &haystack, const std::vector<std::string> &needles) {
  for (const std::string &needle : needles) {
    if (needle != "" && haystack.find(needle) != std::string::npos)
      return true;
  }
  return false;
}

std::string str_replace(const std::string &search , 
                        const std::string &replace, 
                        const std::string &subject, 
                        int32_t count) {
  std::string r{subject};
  int32_t replace_count{0};
  auto it = r.find(search);
  auto search_length = search.length();
  auto replace_length = replace.length();
  while (it != std::string::npos && (-1 == count || replace_count < count)) {
    ++replace_count;
    r.replace(it, search_length, replace);
    it = r.find(search, it + replace_length);
  }
  return r;
}

std::string str_replaces(const std::vector<std::string> &search , 
                         const std::string &replace, 
                         const std::string &subject, 
                         int32_t count) {
  std::string r{subject};
  for (const std::string &item : search)
    r = str_replace(item, replace, subject, count);
  return r;
}

} //namespace string

} //namespace pf_basic
