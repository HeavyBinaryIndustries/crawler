#ifndef MYFUNCTIONS
#define MYFUNCTIONS
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_WORD_LEN 16
#define TITLE_LEN 256
#define URI_LEN 4096
#define MAX_FNAME 256

//スカラー値をコードに変換します。

inline unsigned utf8_scalar_to_code (unsigned source) {
  register unsigned s = source;
  if (s > 0x10ffff) return 0;
  if (s <= 0x7f) {
    return s;
  } else if (s <= 0x7ff) {
    return 0xc080 + (s & 0x3f) + ((s & 0x7c0) << 2);
  } else if (s <= 0xffff) {
    return 0xe08080 + (s & 0x3f) + ((s & 0xfc0) << 2) + ((s & 0xf000) << 4);
  } else if (s <= 0xfffff) {
    return 0xf0808080 + (s & 0x3f) + ((s & 0xfc0) << 2) + ((s & 0x3f000) << 4) + ((s & 0x3c0000) << 6);
  } else {
    return 0;
  }
}

//ascii文字から検索上の意味をなさない文字なら0を返し、大文字アルファベットは小文字に変換します。

inline unsigned convert_ascii(unsigned source) {
  register unsigned s = source;
  if (s <= ' ') {
    return 0;
  } else if (s < 'A') {
    return (s != '\"' && s != '(' && s != ')' && s != '<' && s != '>' && s != ':') * s;
  } else if (s < 'Z') {
    return s - 'A' + 'a';
  } else {
    return (s < 0x7f && s != '[' && s != ']' && s != '{' && s != '}' && s != '|') * s;
  }
}

/*utf-8の3バイト文字から、検索上の意味をなさない文字なら0を返し、全角アルファベットを半角小文字に統一し、
  全角カタカナ、半角ｶﾀｶﾅをひらがなに変換します。*/

inline unsigned convert_utf8_3byte(unsigned source) {
  register unsigned s = source;
  //検索上の意味をなさない文字を除外
  if ((s >= 0xe28098 && s <= 0xe2809f) || (s >= 0xe38080 && s <= 0xe38082) || (s >= 0xe38088 && s <= 0xe38091) || (s >= 0xe38094 && s <= 0xe3809b) || (s == 0xefbc82 || s == 0xefbc88 || s == 0xefbc89 || s == 0xefbc9a || s == 0xefbcbb || s == 0xefbcbd || s == 0xefbd9b || s == 0xefbd9c || s == 0xefbd9d) || (s >= 0xefbd9f && s <= 0xefbda4)) {
    return 0;

  //全角カタカナをひらがなに変換します。
  } else if (s >= 0xe382a1 && s <= 0xe382bf) {
    return s - 0xe382a1 + 0xe38181;
  } else if (s >= 0xe38380 && s <= 0xe3839f) {
    return s - 0xe38380 + 0xe381a0;
  } else if (s >= 0xe383a0 && s <= 0xe383b6) {
    return s - 0xe383a0 + 0xe38280;

  //全角英数を半角小文字に変換します。
  } else if (s >= 0xefbc81 && s <= 0xefbca0) {
    return s - 0xefbc81 + '!';
  } else if (s >= 0xefbca1 && s <= 0xefbcba) {
    return s - 0xefbca1 + 'a';
  } else if (s >= 0xefbd81 && s <= 0xefbd9a) {
    return s - 0xefbd81 + 'a';

  //半角ｶﾀｶﾅをひらがなに変換し、記号も全角に変換します。
  } else if (s >= 0xefbda5 && s <= 0xefbe9f) {
    if (s == 0xefbda5) {
      return 0xe383bb;
    } else if (s == 0xefbda6) {
      return 0xe383b2;
    } else if (s >= 0xefbda7 && s <= 0xefbdab) {
      return (s - 0xefbda7) * 2 + 0xe38181;
    } else if (s >= 0xefbdac && s <= 0xefbdae) {
      return (s - 0xefbdac) * 2 + 0xe38283;
    } else if (s == 0xefbdaf) {
      return 0xe381a3;
    } else if (s == 0xefbdb0) {
      return 0xe383bc;
    } else if (s >= 0xefbdb1 && s <= 0xefbdb5) {
      return (s - 0xefbdb1) * 2 + 0xe38182;
    } else if (s >= 0xefbdb6 && s <= 0xefbdbf) {
      return (s - 0xefbdb6) * 2 + 0xe3818b;
    } else if (s == 0xefbe80) {
      return 0xe3819f;
    } else if (s == 0xefbe81) {
      return 0xe3819f;
    } else if (s >= 0xefbe82 && s <= 0xefbe84) {
      return (s - 0xefbe82) * 2 + 0xe381a4;
    } else if (s >= 0xefbe85 && s <= 0xefbe89) {
      return s - 0xefbe85 + 0xe381aa;
    } else if (s >= 0xefbe8a && s <= 0xefbe8e) {
      return (s - 0xefbe8a) * 3 + 0xe381af;
    } else if (s >= 0xefbe8f && s <= 0xefbe93) {
      return s - 0xefbe8f + 0xe381be;
    } else if (s >= 0xefbe94 && s <= 0xefbe96) {
      return (s - 0xefbe94) * 2 + 0xe38284;
    } else if (s >= 0xefbe97 && s <= 0xefbe9b) {
      return (s - 0xefbe97) * 2 + 0xe38289;
    } else if (s == 0xefbe9c) {
      return 0xe3828f;
    } else if (s == 0xefbe9d) {
      return 0xe38293;
    } else if (s == 0xefbe9e) {
      return 0xe3829b;
    } else if (s == 0xefbe9f) {
      return 0xe3829c;
    } else {
      return s;
    }
  } else {
    return s;
  }
}

inline int strcmp_reg_inline(char* source1, char* source2) {
  register char* s1 = source1;
  register char* s2 = source2;
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *s1 != *s2;
}

//&エスケープをコードに変換し、nに文字数を格納します。

uint32_t solve_and_escape(unsigned char* source, int* n) {
  register unsigned char* s = source;
  register uint32_t reg = 0;
  if (*s != '&') return 0;
  s++;
  if (*s == '#') {
    if (*s == 'x') {
      s += 2;
      int cnt = 0;
      int isdig, issmall, islarge;
      while ((isdig = (*s >= '0' && *s <= '9')) || ((issmall = (*s >= 'a' && *s <= 'f')) || (islarge = (*s >= 'A' && *s <= 'F'))) && cnt <= 8) {
        reg <<= 4;
        if (isdig) {
          reg |= *s - '0';
        } else if (issmall) {
          reg |= *s - 'a' + 10;
        } else {
          reg |= *s - 'A' + 10;
        }
        cnt++;
        s++;
      }
      if (*s == ';') {
        *n = cnt + 4;
        return utf8_scalar_to_code(reg);
      }
    } else if (s[2] >= '0' && s[2] <= '9') {
      s++;
      int cnt = 0;
      while (*s >= '0' && *s <= '9' && cnt <= 10) {
        reg *= 10;
        reg += *s - '0';
        cnt++;
        s++;
      }
      if (*s == ';') {
        *n = cnt + 3;
        return utf8_scalar_to_code(reg);
      }
    }
  } else if ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z')) {
    char buf[11];
    int cnt = 0;
    while (*s >= 'a' && *s <= 'z' && cnt < 10) {
      buf[cnt++] = *(s++);
    }
    buf[cnt] = '\0';
    *n = cnt + 2;
    if (*s == ';') {
      if (!strcmp_reg_inline(buf, "nbsp")) {
        return ' ';
      } else if (!strcmp_reg_inline(buf, "amp")) {
        return '&';
      } else if (!strcmp_reg_inline(buf, "quat")) {
        return '\"';
      } else if (!strcmp_reg_inline(buf, "apos")) {
        return '\'';
      } else if (!strcmp_reg_inline(buf, "minus")) {
        return '-';
      } else if (!strcmp_reg_inline(buf, "lt")) {
        return '<';
      } else if (!strcmp_reg_inline(buf, "gt")) {
        return '>';
      } else if (!strcmp_reg_inline(buf, "sim")) {
        return '~';
      } else if (!strcmp_reg_inline(buf, "lowast")) {
        return '*';
      } else if (!strcmp_reg_inline(buf, "yen")) {
        return 0xc2a5;
      } else if (!strcmp_reg_inline(buf, "cent")) {
        return 0xc2a2;
      } else if (!strcmp_reg_inline(buf, "pound")) {
        return 0xc2a3;
      } else if (!strcmp_reg_inline(buf, "euro")) {
        return 0xc2a4;
      } else if (!strcmp_reg_inline(buf, "copy")) {
        return 0xc2a9;
      } else if (!strcmp_reg_inline(buf, "reg")) {
        return 0xc2ae;
      } else if (!strcmp_reg_inline(buf, "trade")) {
        return 0xe284a2;
      }
    }
  }
  *n = 0;
  return 0;
}

//改行の間で共通の文を除去します。

void remove_same_string(char* source, char* cmp) {
  register char* s = source;
  register char* d = s;
  register char* c = cmp;
  char* base_d = d;
  char* base_c = c;
  char* end_d;
  int eq;
  while (*s) {
    base_d = d;
    while (*s && *s != '\n') {
      *(d++) = *(s++);
    }
    s += *s == '\n';
    *(d++) = '\n';
    end_d = d;
    eq = 0;
    c = base_c;
    while (*c && !eq) {
      eq = 1;
      d = base_d;
      while (*c && *c != '\n' && *d != '\n' && eq) {
        eq &= *(d++) == *(c++);
      }
      eq &= *d == *c;
      if (*c != '\n' && *c) {
        eq = 0;
        while (*c != '\n' && *c) {
          c++;
        }
      }
      c += *c == '\n';
    }
    if (eq) {
      d = base_d;
    } else {
      d = end_d;
    }
  }
  *d = '\0';
}

struct http_buf {
  char* data;
  unsigned data_size;
};

#define BUF_SIZE 0xffffff

size_t buf_write(char* ptr, size_t size, size_t nmemb, void* data) {
  struct http_buf* buf = data;
  int block = size * nmemb;
  if (buf == NULL) return block;
  if (block + 1 >= BUF_SIZE - buf->data_size) block = BUF_SIZE - buf->data_size - 1;
  if (block) {
    if (buf->data == NULL) {
      buf->data = malloc(BUF_SIZE);
      if (buf->data == NULL) exit(1);
    }
    memcpy(buf->data + buf->data_size, ptr, block);
    buf->data[buf->data_size + block] = '\0';
    buf->data_size += block;
  }
  return block;
}

//sourceで指定した文字列からuint32に変換します。

inline uint32_t code_to_uint_utf8(unsigned char* source) {
  register unsigned char* s = source;
  if (s[0] <= 0x7f) {
    return s[0];
  } else if (s[0] <= 0xbf) {
    return 0;
  } else if (s[0] <= 0xdf) {
    return (s[0] << 8) | s[1];
  } else if (s[0] <= 0xef) {
    return (s[0] << 16) | (s[1] << 8) | s[2];
  } else if (s[0] <= 0xf7) {
    return (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
  } else {
    return 0;
  }
}

//"source"からutf-8文字一文字をdestに書き込み、ヌル終端させ、文字数を返します。

inline int pick_char_utf(unsigned char* source, unsigned char* dest) {
  register unsigned char* s = source;
  register unsigned char* d = dest;
  if (s[0] <= ' ') {
    *d = '\0';
    return 0;
  } else if (s[0] <= 0x7f) {
    *(d++) = *s;
    *d = '\0';
    return 1;
  } else if (s[0] <= 0xbf) {
    *d = '\0';
    return 0;
  } else if (s[0] <= 0xdf) {
    *(d++) = *(s++);
    *(d++) = *(s++);
    *d = '\0';
    return 2;
  } else if (s[0] <= 0xef) {
    *(d++) = *(s++);
    *(d++) = *(s++);
    *(d++) = *(s++);
    *d = '\0';
    return 3;
  } else if (s[0] <= 0xf7) {
    *(d++) = *(s++);
    *(d++) = *(s++);
    *(d++) = *(s++);
    *(d++) = *(s++);
    *d = '\0';
    return 4;
  } else {
    *d = '\0';
    return 0;
  }
}

//"source"からutf-8一文字を16進数に変換しdestにヌル終端させ、utf-8文字のバイト数を返します。

inline int code_to_hexagonal_string_utf8(unsigned char* source, unsigned char* dest) {
  register unsigned char* s = source;
  register unsigned char* d = dest;
  register unsigned reg;
  if (s[0] <= ' ') {
    *d = '\0';
    return 0;
  } else if (s[0] <= 0x7f) {
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    *d = '\0';
    return 1;
  } else if (s[0] <= 0xbf) {
    *d = '\0';
    return 0;
  } else if (s[0] <= 0xdf) {
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    s++;
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    *d = '\0';
    return 2;
  } else if (s[0] <= 0xef) {
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    s++;
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    s++;
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    *d = '\0';
    return 3;
  } else if (s[0] <= 0xf7) {
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    s++;
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    s++;
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    s++;
    reg = *s >> 4;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    reg = *s & 0xf;
    *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
    *d = '\0';
    return 4;
  } else {
    *d = '\0';
    return 0;
  }
}

//utf-8文字列"source"を16進数に変換し、destにヌル終端して格納します。

inline int utf8_string_to_hexagonal_path(unsigned char* source, unsigned char* dest) {
  register unsigned char* s = source;
  register unsigned char* d = dest;
  unsigned char* end = s + strlen(s);

  while (s < end) {
    register unsigned reg;
    if (s[0] <= 0x7f) {
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
    } else if (s[0] <= 0xbf) {
      *d = '\0';
      return 0;
    } else if (s[0] <= 0xdf) {
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
    } else if (s[0] <= 0xef) {
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
    } else if (s[0] <= 0xf7) {
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
      reg = *s >> 4;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      reg = *s & 0xf;
      *(d++) = (reg <= 0x9) * ('0' + reg) + (reg >= 0xa) * ('a' + reg - 0xa);
      s++;
    } else {
      *dest = '\0';
      return 1;
    }
  }
  *d = '\0';
  return 0;
}

//utf-8文字列を検索用に変換します。

int convert_utf8(unsigned char* source, unsigned char* dest) {
  register unsigned char* s = source;
  unsigned char* end = s + strlen(s);
  unsigned char* d = dest;
  while (s < end) {
    register uint32_t reg = 0;
    if (*s <= ' ') {
      s++;
      continue;
    } else if (*s <= 0x7f) {
      reg = convert_ascii(*(s++));
    } else if (*s <= 0xbf) {
      *dest = '\0';
      return 1;
    } else if (*s <= 0xdf) {
      reg = (*(s++) << 8) | *(s++);
    } else if (*s <= 0xef) {
      reg = convert_utf8_3byte((*(s++) << 16) | (*(s++) << 8) | *(s++));
    } else if (*s <= 0xf7) {
      reg = (*(s++) << 24) | (*(s++) << 16) | (*(s++) << 8) | *(s++);
    } else {
      *d = '\0';
      return 1;
    }
    if (reg > 0xffffff) {
      *(d++) = reg >> 24;
    }
    if (reg > 0xffff) {
      *(d++) = (reg >> 16) & 0xff;
    }
    if (reg > 0xff) {
      *(d++) = (reg >> 8) & 0xff;
    }
    if (reg) {
      *(d++) = reg & 0xff;
    }
  }
  *d = '\0';
  return 0;
}

int strncmp_alphabet(char* str1, char* str2, int n) {
  char* buf1 = malloc(n);
  char* buf2 = malloc(n);
  for (int i = 0; i < n; i++) {
    buf1[i] = tolower(str1[i]);
    buf2[i] = tolower(str2[i]);
  }
  n = strncmp(buf1, buf2, n);
  free(buf1);
  free(buf2);
  return n;
}

//*や$を含む文字列とパスを比較し、文字列とパスが対応する場合、0を返します。

int strpathcmp(char* string, char* path) {
  int end_s = strlen(string);
  int end_p = strlen(path);
  if (end_p < end_s) return 1;
  if (string[end_s - 1] == '$') {
    int i;
    for (i = 0; i <= end_s - 2; i++) {
      if (string[end_s - 2 - i] == '*') break;
      if (string[end_s - 2 - i] != path[end_p - 1 - i]) return 1;
    }
    end_s = end_s - 2 - i;
  }
  int j = 0;
  for (int i = 0; i < end_s; i++) {
    if (string[i] == '*') {
      if (i + 1 >= end_s) {
        return 0;
      } else {
        i++;
        int n;
        for (n = 0; n < end_s - i; n++) {
          if (string[i + n] == '*') break;
        }
        while (j < end_p) {
          if (!strncmp(path + j, string + i, n)) break;
          j++;
        }
        if (j >= end_p) return 1;
      }
    } else {
      if (path[j] != string[i]) {
        return 1;
      }
    }
    j++;
  }
  return 0;
}

//16進数文字列から文字列に変換します。

void hexagonal_to_string(unsigned char* source, unsigned char* dest) {
  unsigned char* s = source;
  unsigned char* end = s + strlen(s);
  unsigned char* d = dest;
  while (s < end) {
    if (s[0] < '0' || (s[0] > '9' && s[0] < 'a') || s[0] > 'f') break;
    if (s[1] < '0' || (s[1] > '9' && s[1] < 'a') || s[1] > 'f') break;
    *d = (((s[0] <= '9') * (s[0] - '0') + (s[0] >= 'a') * (s[0] - 'a' + 10)) << 4)
          | (s[1] <= '9') * (s[1] - '0') + (s[1] >= 'a') * (s[1] - 'a' + 10);
    s += 2;
    d++;
  }
  *d = '\0';
}

//文字列集合を保存する構造体です。

struct str_aggregate {
  char* base;
  unsigned current_size;
  unsigned max_size;
};

//文字列を追加します

int add_to_aggregate(struct str_aggregate* list, char* str) {
  int size = strlen(str);
  if (size + 1 < list->max_size - list->current_size) {
    register char* d = list->base + list->current_size;
    register char* s = str;
    while (*s != '\0') {
      *(d++) = *(s++);
    }
    *(d++) = '\0';
    list->current_size += size + 1;
    return 0;
  }
  return 1;
}

//対象の文字列がある場合、1を返します。

int is_in_aggregate(struct str_aggregate* list, char* str) {
  register char* s = str;
  register char* c = list->base;
  char* end = list->base + list->current_size;
  while (c < end) {
    s = str;
    while(*c == *s && *c && *s) {
      c++;
      s++;
    }
    if (*c == *s) {
      return 1;
    }
    while(*c) {
      c++;
    }
    c++;
  }
  return 0;
}

//文字列を探し、見つかった場合、文字列の番号を返します。

unsigned search_in_aggregate(struct str_aggregate* list, char* str) {
  register char* s = str;
  register char* c = list->base;
  char* end = list->base + list->current_size;
  unsigned n = 0;
  while (c < end) {
    s = str;
    while(*c == *s && *c && *s) {
      c++;
      s++;
    }
    if (*c == *s) {
      return n;
    }
    while(*c) {
      c++;
    }
    c++;
    n++;
  }
  return 0xffffffff;
}

//文字列とパスを比較します

int is_adapt_formats(struct str_aggregate* list, char* link) {
  char* base = list->base;
  char* end = base + list->current_size;
  while (base < end) {
    if (!strpathcmp(base, link)) return 1;
    base += strlen(base) + 1;
  }
  return 0;
}

#endif
