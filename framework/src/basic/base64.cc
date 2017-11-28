#include "pf/basic/base64.h"

namespace pf_basic {

char getb64char(int index) {
  const char kBase64Table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  if ((index >= 0) && (index < 64)) {
      return kBase64Table[index];
  }
  return '=';
}

//从双字中取单字节
#define B0(a) (a & 0xFF)
#define B1(a) (a >> 8 & 0xFF)
#define B2(a) (a >> 16 & 0xFF)
#define B3(a) (a >> 24 & 0xFF)

//编码后的长度一般比原文多占1/3的存储空间，请保证base64code有足够的空间
int base64encode(char *base64code, const char *src, int src_len) {
  unsigned char *psrc = (unsigned char*)src;
  char *p64 = base64code;
  int len = 0;
  int i = 0;
  int b0 = 0;
  int b1 = 0;
  int b2 = 0;
  int b3 = 0;
  
  if (src_len == 0) {
      src_len = static_cast<int>(strlen(src));
  }
  
  for (i = 0; i < src_len - 3; i += 3){
    unsigned long ulTmp = *(unsigned long*)psrc;
    b0 = getb64char((B0(ulTmp) >> 2) & 0x3F); 
    b1 = getb64char((B0(ulTmp) << 6 >> 2 | B1(ulTmp) >> 4) & 0x3F); 
    b2 = getb64char((B1(ulTmp) << 4 >> 2 | B2(ulTmp) >> 6) & 0x3F); 
    b3 = getb64char((B2(ulTmp) << 2 >> 2) & 0x3F); 
    *((unsigned long*)p64) = b0 | b1 << 8 | b2 << 16 | b3 << 24;
    len += 4;
    p64 += 4; 
    psrc += 3;
  }
  
  //处理最后余下的不足3字节的饿数据
  if (i < src_len) {
    int rest = src_len - i;
    unsigned long ulTmp = 0;
    for (int j = 0; j < rest; ++j) {
      *(((unsigned char*)&ulTmp) + j) = *psrc++;
    }
    
    p64[0] = getb64char((B0(ulTmp) >> 2) & 0x3F); 
    p64[1] = getb64char((B0(ulTmp) << 6 >> 2 | B1(ulTmp) >> 4) & 0x3F); 
    p64[2] = rest > 1 ? 
      getb64char((B1(ulTmp) << 4 >> 2 | B2(ulTmp) >> 6) & 0x3F) : '='; 
    p64[3] = rest > 2 ? getb64char((B2(ulTmp) << 2 >> 2) & 0x3F) : '='; 
    p64 += 4; 
    len += 4;
  }
  
  *p64 = '\0'; 
  
  return len;
}

int getb64index(char ch) {
  int index = -1;
  
  if (ch >= 'A' && ch <= 'Z') {
    index = ch - 'A';
  }
  else if (ch >= 'a' && ch <= 'z') {
    index = ch - 'a' + 26;
  }
  else if (ch >= '0' && ch <= '9') {
    index = ch - '0' + 52;
  }
  else if (ch == '+') {
    index = 62;
  }
  else if (ch == '/') {
    index = 63;
  }

  return index;
}

//解码后的长度一般比原文少用占1/4的存储空间，请保证buf有足够的空间
int base64decode(char * buf, const char * base64code, int src_len) { 
  int ii = 0;
  int jj = 0;
  int len = 0;
  char * pbuf = buf;
  int b0 = 0;
  int b1 = 0;
  int b2 = 0;
  //int b3 = 0;
  int rest = 0;
  unsigned long ulTmp = 0;
  unsigned char* psrc = (unsigned char*)base64code;
  
  if (src_len == 0)
    src_len = static_cast<int>(strlen(base64code));

  for (ii = 0; ii < src_len - 4; ii += 4) {
    ulTmp = *(unsigned long*)psrc;
    
     b0 = (getb64index((char)B0(ulTmp)) << 2 | 
         getb64index((char)B1(ulTmp)) << 2 >> 6) & 0xFF;
     b1 = (getb64index((char)B1(ulTmp)) << 4 | 
         getb64index((char)B2(ulTmp)) << 2 >> 4) & 0xFF;
     b2 = (getb64index((char)B2(ulTmp)) << 6 | 
         getb64index((char)B3(ulTmp)) << 2 >> 2) & 0xFF;
    
    *((unsigned long*)pbuf) = b0 | b1 << 8 | b2 << 16;
    psrc  += 4; 
    pbuf += 3;
    len += 3;
  }

  //处理最后余下的不足4字节的饿数据
  if (ii < src_len) {
    rest = src_len - ii;
    ulTmp = 0;
    for (jj = 0; jj < rest; ++jj) {
      *(((unsigned char*)&ulTmp) + jj) = *psrc++;
    }
    
    b0 = (getb64index((char)B0(ulTmp)) << 2 | 
        getb64index((char)B1(ulTmp)) << 2 >> 6) & 0xFF;
    *pbuf++ = static_cast<char>(b0);
    len++;

    if ('=' != B1(ulTmp) && '=' != B2(ulTmp)) {
       b1 = (getb64index((char)B1(ulTmp)) << 4 | 
           getb64index((char)B2(ulTmp)) << 2 >> 4) & 0xFF;
      *pbuf++ = static_cast<char>(b1);
      len  ++;
    }
    
    if ('=' != B2(ulTmp) && '=' != B3(ulTmp)) {
       b2 = (getb64index((char)B2(ulTmp)) << 6 | 
           getb64index((char)B3(ulTmp)) << 2 >> 2) & 0xFF;
      *pbuf++ = static_cast<char>(b2);
      len++;
    }
  }
    
  *pbuf = '\0';
  return len;
} 

} //namespace pf_basic
