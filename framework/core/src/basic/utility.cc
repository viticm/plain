#include "plain/basic/utility.h"
#include <random>
#if OS_UNIX
#include <sys/stat.h>
#include <sys/resource.h>
#endif
#include "plain/basic/base64.h"
#include "plain/basic/io.h"
#include "plain/sys/assert.h"

namespace plain {

char g_error_buff[512]{0};

/*
 This function form muduo.
 Format a number with 5 characters, including SI units.
 [0,     999]
 [1.00k, 999k]
 [1.00M, 999M]
 [1.00G, 999G]
 [1.00T, 999T]
 [1.00P, 999P]
 [1.00E, inf)
 @refer:
*/
std::string format_size(uint64_t s) {
  double n = static_cast<double>(s);
  // FIXME: use std::format
  char buf[64];
  if (s < 1000)
    snprintf(buf, sizeof(buf), "%" PRId64, s);
  else if (s < 9995)
    snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
  else if (s < 99950)
    snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
  else if (s < 999500)
    snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
  else if (s < 9995000)
    snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
  else if (s < 99950000)
    snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
  else if (s < 999500000)
    snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
  else if (s < 9995000000)
    snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
  else if (s < 99950000000)
    snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
  else if (s < 999500000000)
    snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
  else if (s < 9995000000000)
    snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
  else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
  else if (s < 999500000000000)
    snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
  else if (s < 9995000000000000)
    snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
  else if (s < 99950000000000000)
    snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
  else if (s < 999500000000000000)
    snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
  else
    snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
  return buf;
}

char get_base64char(int32_t index) {
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
  int32_t i, j;
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
#ifdef PLAIN_OPEN_ICONV
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
        int32_t one = 1 ;
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
  io_cwarn("pf_basic::charset_convert not work without iconv.");
#endif
  return 0;
}
#endif //PLAIN_OPEN_ICONV

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
                std::vector<std::string>& result,
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
  int32_t size{0};
  int32_t index{0};
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
  int32_t i{0}, c{0}, cc{0};
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

  int32_t prime_number_list[PG_RESULTLENSTD] = {
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
  auto tickcount{1}; // = TIME_MANAGER_POINTER->get_tickcount();
  char temp[PG_RESULTLENSTD + 1]{0};
  int32_t level = 0;
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

std::string str_replaces(const std::vector<std::string>& search , 
                         const std::string &replace, 
                         const std::string &subject, 
                         int32_t count) {
  std::string r{subject};
  for (const std::string &item : search)
    r = str_replace(item, replace, subject, count);
  return r;
}

std::string wstr2str(const std::wstring &str) {
  std::string r;
#if OS_WIN
  auto wText = str.c_str();
  DWORD dwNum =
    WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
  char *psText = new char[dwNum];
  WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
  r = psText;
  safe_delete_array(psText);
#else
  char temp[512]{0,};
  wcstombs(temp, str.c_str(), sizeof(temp) - 1);
  r = temp;
#endif
  return r;
}

std::wstring str2wstr(const std::string &str) {
  std::wstring r;
#if OS_WIN
  int32_t len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), -1, NULL, 0);
  wchar_t *wszUtf8 = new wchar_t[len + 1];
  memset(wszUtf8, 0, len * 2 + 2);
  MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), -1, (LPWSTR)wszUtf8, len);
  r = wszUtf8;
  safe_delete_array(wszUtf8);
#else
  wchar_t temp[512]{0,};
  mbstowcs(temp, str.c_str(), sizeof(temp) - 1);
  r = temp;
#endif
  return r;
}

static char ascii_table[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8',
  '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

char value_toascii(char in) {
  char out =
    0 < in && sizeof(ascii_table) / sizeof(char) >
    static_cast< size_t >(in) ? ascii_table[(uint8_t)in] : '?';
  return out;
}

char ascii_tovalue(char in) {
  char out;
  switch(in) {
    case '0': {
      out = 0;
      break;
    }
    case '1': {
      out = 1;
      break;
    }
    case '2': {
      out = 2;
      break;
    }
    case '3': {
      out = 3;
      break;
    }
    case '4': {
      out = 4;
      break;
    }
    case '5': {
      out = 5;
      break;
    }
    case '6': {
      out = 6;
      break;
    }
    case '7': {
      out = 7;
      break;
    }
    case '8': {
      out = 8;
      break;
    }
    case '9': {
      out = 9;
      break;
    }
    case 'A': {
      out = 10;
      break;
    }
    case 'B': {
      out = 11;
      break;
    }
    case 'C': {
      out = 12;
      break;
    }
    case 'D': {
      out = 13;
      break;
    }
    case 'E': {
      out = 14;
      break;
    }
    case 'F': {
      out = 15;
      break;
    }
    default: {
      out = '?';
      Assert(false);
    }
  }
  return out;
}

bool binary_tostring(const char *in, uint32_t in_length, char *out) {
  if (0 == in_length) return false;
  uint32_t out_index = 0;
  uint32_t i;
  for (i = 0; i < in_length; ++i) {
    out[out_index] = value_toascii((static_cast<uint8_t>(in[i] & 0xF0)) >> 4);
    ++out_index;
    out[out_index] = value_toascii(in[i] & 0xF0);
    ++out_index;
  }
  return true;
}

bool string_tobinary(const char *in,
                     uint32_t in_length,
                     char *out,
                     uint32_t out_limit,
                     uint32_t &out_length) {
  if (0 == in_length) return false;
  uint32_t out_index = 0;
  uint32_t i;
  for (i = 0; i < in_length; ++i) {
    if ('\0' == in[i] || '\0' == in[i]) break;
    out[out_index] = (ascii_tovalue(in[i]) << 4) + ascii_tovalue(in[i + 1]);
    ++out_index;
    i += 2;
    if (out_index > out_limit) break;
  }
  out_length = out_index; //length must be the out_limit
  return true;
}

void dirname(const char *filepath, char *save) {
    if (nullptr == filepath || nullptr == save) return;
    char _filepath[FILENAME_MAX] = {0};
    safecopy(_filepath, filepath, sizeof(_filepath));
    uint32_t copysize = 0;
    const char *find = nullptr;
    path_tounix(_filepath, sizeof(_filepath));
    find = strrchr(_filepath, '/');
    if (find) copysize = static_cast<uint32_t>(find - _filepath);
    safecopy(save, _filepath, copysize + 1);
}

void sleep(uint32_t million_seconds) {
#if OS_WIN
  Sleep(million_seconds);
#elif OS_UNIX
  usleep(million_seconds * 1000);
#endif
}

void get_sizestr(uint64_t size, char *buffer, uint32_t length, int8_t type) {
  int8_t realtype = 0;
  if (0 == type) {
    snprintf(buffer, length, "%.2fbytes", size / 1.0);
    return;
  }
  float lastsize = size / 1.0f;
  float floatsize = size / 1.0f;
  float finalsize = .0f;
  for (int8_t i = -1 == type ? 4 : type; i > 0; --i) {
    lastsize = floatsize;
    floatsize /= 1024;
    ++realtype;
    if ((floatsize < 1.0f && -1 == type) || floatsize < 0.01f) break;
  }
  if ((floatsize < 1.0f && -1 == type) || floatsize < 0.01f) {
    realtype -= 1;
    finalsize = lastsize;
  }
  else {
    finalsize = floatsize;
  }
  switch (realtype) {
    case 0: {
      snprintf(buffer, length, "%.2fbytes", finalsize);
      break;
    }
    case 1: {
      snprintf(buffer, length, "%.2fkb", finalsize);
      break;
    }
    case 2: {
      snprintf(buffer, length, "%.2fmb", finalsize);
      break;
    }
    case 3: {
      snprintf(buffer, length, "%.2fgb", finalsize);
      break;
    }
    case 4: {
      snprintf(buffer, length, "%.2ft", finalsize);
      break;
    }
    default: {
      snprintf(buffer, length, "%.2funkown", finalsize);
      break;
    }
  }
}

void path_tounix(char *buffer, uint16_t length) {
  for (uint16_t i = 0; i < length; ++i) {
    if ('\\' == buffer[i]) buffer[i] = '/';
  }
}

void path_towindows(char *buffer, uint16_t length) {
  for (uint16_t i = 0; i < length; ++i) {
    if ('/' == buffer[i]) buffer[i] = '\\';
  }
}

void get_module_filename(char *buffer, size_t size) {
  int32_t resultcode = 0;
#if OS_WIN
#ifdef _UNICODE
  resultcode = (int32_t)GetModuleFileName(
		nullptr, reinterpret_cast<LPWSTR>(buffer), (DWORD)size);
#else
  resultcode = (int32_t)GetModuleFileName(nullptr, buffer, (DWORD)size);
#endif
#elif OS_UNIX
  resultcode = readlink("/proc/self/exe", buffer, size);
  Assert(resultcode > 0 && resultcode < static_cast<int32_t>(size));
  buffer[resultcode] = '\0';
#endif
}

void disable_windowclose() {
#if OS_WIN
  HWND hwnd = GetConsoleWindow();
  if (hwnd) {
    HMENU hMenu = GetSystemMenu( hwnd, FALSE );
    EnableMenuItem(hMenu, SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);
  }
#endif
}

#if OS_UNIX
bool makedir(const char *path, uint16_t mode) {
#elif OS_WIN
bool makedir(const char *path, uint16_t) {
#endif
  char _path[FILENAME_MAX] = {0};
  int32_t i = 0;
  int32_t result = 0;
  int32_t length = static_cast<int32_t>(strlen(path));
  safecopy(_path, path, sizeof(_path));
  path_tounix(_path, static_cast<uint16_t>(length));
  if (_path[length - 1] != '/') {
    _path[length] = '/';
    _path[length + 1] = '\0';
  }
  result = access(_path, 0);
  if (0 == result) return true;
  int32_t _length = static_cast<int32_t>(strlen(_path));
  for (i = 0; i < _length; ++i) {
    if ('/' ==  _path[i]) {
      _path[i] = '\0';
      if (0 == strlen(_path)) {
        _path[i] = '/';
        continue;
      }
      result = access(_path, 0);
      if (result != 0 && mkdir(_path, mode) != 0) {
        return false;
      }
      _path[i] = '/';
    }
  }
  return true;
}

uint64_t touint64(uint32_t high, uint32_t low) {
  uint64_t value = high;
  value = (value << 32) | low;
  return value;
}

uint32_t get_highsection(uint64_t value) {
  uint32_t result = static_cast<uint32_t>((value >> 32) & 0xFFFFFFFF);
  return result;
}

uint32_t get_lowsection(uint64_t value) {
  uint32_t result = static_cast<uint32_t>(value & 0xFFFFFFFF);
  return result;
}

void complementpath(char *filepath, size_t size, char delimiter) {
  uint32_t length = static_cast<uint32_t>(strlen(filepath));
  if (size <= length) return;
  filepath[length] = delimiter == filepath[length - 1] ? '\0' : delimiter;
}

char *strerror_pl(int32_t saved_errno) {
#if OS_UNIX
  return strerror_r(saved_errno, g_error_buff, sizeof g_error_buff);
#else
  strerror_s(g_error_buff, sizeof g_error_buff, saved_errno);
  return g_error_buff;
#endif
}

std::string get_error_str(int32_t errno) {
  char buf[1024];
  buf[0] = '\0';
#if OS_WIN
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, errno, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPWSTR>(buf), sizeof(buf), NULL);
#else

#ifdef _GNU_SOURCE

#if !defined(__GLIBC__)
  int e = strerror_r(errno, buf, sizeof(buf));
  auto s = strerror(e);
  return s ? std::string(s) : std::string{};
#else
  auto s = strerror_r(errno, buf, sizeof(buf));
  return s ? std::string(s) : std::string{};
#endif

#endif // _GNU_SOURCE

#endif
  return buf;
}

} // namespace plain
