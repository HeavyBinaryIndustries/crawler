#ifndef EXTRACT_PL_H
#define EXTRACT_PL_H

#include <string.h>
#include "myfunctions.h"

/*"source"で指定したhtml文字列から、plainに平文を抽出し、utf-8に従ってカタカナ、半角ｶﾀｶﾅをひらがなに、大文字アルファベットを小文字アルファベット、
  全角アルファベットを半角小文字のアルファベットに変換し、検索上の意味をなさないかっこや全角スペースなどは半角スペースに変換します。*/
//<script>,<style>タグ内以外の文章を抽出します
//</p>タグ、</li>タグで改行します。
/*"title"にhtmlのタイトルを検索用に処理したものを、"raw_title"にタイトルの原文をTITLE_LENまで書き込みます*/
/*href属性でリンク付けされているリンクを抽出し、
  内部リンクを"internal_link"、外部リングを"outer_link"に、それぞれ重複がないかを検査したうえで追加します*/

int extract_plaintext_and_linklist(unsigned char* source, unsigned char* plain, unsigned char* title, unsigned char* raw_title,
                                    unsigned char* scheme, unsigned char* host, unsigned char* file_path,
                                    struct str_aggregate* internal_link, struct str_aggregate* outer_link) {
  unsigned source_len = strlen(source);
  register unsigned char* s = source;
  register unsigned char* end = s + source_len;
  register unsigned reg = 0;
  unsigned href = 'h' << 24 | 'r' << 16 | 'e' << 8 | 'f';
  int hyperlink = 0;
  int script = 0;
  int style = 0;
  *plain = '\n';
  if (strcmp(scheme, "http") && strcmp(scheme, "https")) {
    return 1;
  }
  unsigned scheme_len = strlen(scheme);
  unsigned host_len = strlen(host);
  unsigned file_path_len = strlen(file_path);
  if (scheme_len + host_len + file_path_len > URI_LEN) return 1;
  unsigned char* scheme_host_file_path = malloc(URI_LEN);
  unsigned char* uri_buf = malloc(URI_LEN);
  if (!scheme_host_file_path || !uri_buf) {
    puts("extract_plaintext_and_linklist: failed to alloc memory");
    exit(1);
  }
  sprintf(scheme_host_file_path, "%s://%s%s", scheme, host, file_path);
  int nofollow;
  unsigned char* base;
  unsigned upper_direct;
  int hit;
  unsigned char* p;
  unsigned issmall, islarge, isdig;
  int cnt;
  int first = 1;
  int dirdepth = 0;
  int insite, offset, ignore;
  for (int i = 0; i < file_path_len; i++) {
    dirdepth += file_path[i] == '/';
    if (file_path[i] == '/' && file_path[i + 1] == '/') return 1;
  }

  while (s < end) {
    if (*s != '<') {
      while (s < end && *s != '<') {
        if (s[0] == '&') {
          int n;
          reg = solve_and_escape(s, &n);
          if (!n) {
            *(plain++) = *(s++);
            continue;
          }
          s += n;
          if (reg <= 0x7f) {
            reg = convert_ascii(reg);
          } else if (reg > 0xffff && reg <= 0xffffff) {
            reg = convert_utf8_3byte(reg);
          }
          if (reg > 0xffffff) {
            *(plain++) = (reg >> 24) & 0xff;
          }
          if (reg > 0xffff) {
            *(plain++) = (reg >> 16) & 0xff;
          }
          if (reg > 0xff) {
            *(plain++) = (reg >> 8) & 0xff;
          }
          if (reg > 0) {
            first = 0;
            *(plain++) = reg & 0xff;
          } else {
            if (!first && plain[-1] > ' ') *(plain++) = ' ';
          }
        }
        if (s[0] <= 0x7f) {
          reg = s[0];
          reg = convert_ascii(reg);
          if (reg) {
            *(plain++) = reg & 0xff;
          } else {
            if (!first && plain[-1] > ' ') *(plain++) = ' ';
          }
          s++;
        } else if (s[0] <= 0xbf) {
          s++;
        } else if (s[0] <= 0xdf) {
          first = 0;
          *(plain++) = *(s++);
          *(plain++) = *(s++);
        } else if (s[0] <= 0xef) {
          reg = (s[0] << 16) | (s[1] << 8) | s[2];
          reg = convert_utf8_3byte(reg);
          if (reg > 0xffffff) {
            *(plain++) = (reg >> 24) & 0xff;
          }
          if (reg > 0xffff) {
            *(plain++) = (reg >> 16) & 0xff;
          }
          if (reg > 0xff) {
            *(plain++) = (reg >> 8) & 0xff;
          }
          if (reg > 0) {
            first = 0;
            *(plain++) = reg & 0xff;
          } else {
            if (!first && plain[-1] > ' ') *(plain++) = ' ';
          }
          s += 3;
        } else if (s[0] <= 0xf7){
          first = 0;
          *(plain++) = *(s++);
          *(plain++) = *(s++);
          *(plain++) = *(s++);
          *(plain++) = *(s++);
        } else {
          s++;
          while (*s >= 0x80 && *s <= 0xbf) s++;
        }
      }
      if (!first && plain[-1] > ' ') {
        *(plain++) = ' ';
      }
    } else if (*(s + 1) == '!' && *(s + 2) == '-') {
      reg = *(s + 3) == '-';
      s += 4;
      if (reg) {
        while (!(*s == '-' && *(s + 1) == '-') && s < end) s++;
      } else {
        while (*s != '-' && s < end) s++;
      }
      while (*s != '>' && s < end) s++;
      s++;
    } else {
      s++;
      reg = 0;
      while (s[reg] != '>' && s + reg < end) {
        while (s[reg] != '>' && s[reg] != '\"' && s + reg < end) {
          s[reg] += (s[reg] >= 'A' && s[reg] <= 'Z') * ('a' - 'A');
          reg++;
        }
        if (s[reg] == '\"') {
          reg++;
          while (s[reg] != '>' && s[reg] != '\"' && s + reg < end) reg++;
        }
      }
      script = !strncmp(s, "script", 6);
      style = !strncmp(s, "style", 5);
      if (!strncmp(s, "title", 5)) {
        unsigned i = 0;
        unsigned j = 0;
        while (s[i] != '>') i++;
        i++;
        for (j = 0; j < TITLE_LEN - 1; j++) {
          if (s[i + j] == '<') break;
          if (s[i + j] == '&') {
            int n;
            reg = solve_and_escape(s + i + j, &n);
            j += n;
            if (reg == ',') continue;
            if (reg > 0xffffff) {
              raw_title[j++] = (reg >> 24) & 0xff;
            }
            if (reg > 0xffff) {
              raw_title[j++] = (reg >> 16) & 0xff;
            }
            if (reg > 0xff) {
              raw_title[j++] = (reg >> 8) & 0xff;
            }
            if (reg > 0) {
              raw_title[j++] = reg & 0xff;
            } else {
              if (j && plain[-1] > ' ') raw_title[j++] = ' ';
            }
            continue;
          }
          raw_title[j] = s[i + j];
        }
        raw_title[j] = '\0';
        int k = 0;
        int l = 0;
        while (k < j) {
          if (raw_title[k] <= 0x7f) {
            reg = raw_title[k++];
            reg = convert_ascii(reg);
          } else if (raw_title[k] <= 0xbf) {
            k++;
            continue;
          } else if (raw_title[k] <= 0xdf) {
            reg = (raw_title[k++] << 8) | raw_title[k++];
          } else if (raw_title[k] <= 0xef) {
            reg = (raw_title[k++] << 16) | (raw_title[k++] << 8) | raw_title[k++];
            reg = convert_utf8_3byte(reg);
          } else if (raw_title[k] <= 0xf7) {
            reg = (raw_title[k++] << 24) | (raw_title[k++] << 16) | (raw_title[k++] << 8) | raw_title[k++];
          } else {
            k += 5 + (raw_title[k] >= 0xfc);
            continue;
          }
          if (reg > 0xffffff) {
            title[l++] = (reg >> 24) & 0xff;
          }
          if (reg > 0xffff) {
            title[l++] = (reg >> 16) & 0xff;
          }
          if (reg > 0xff) {
            title[l++] = (reg >> 8) & 0xff;
          }
          if (reg > 0) {
            title[l++] = reg & 0xff;
          } else {
            if (l && title[l - 1] > ' ') title[l++] = ' ';
          }
        }
        title[l] = '\0';
      }
      if (s[0] == '/' && ((s[1] == 'p' && (s[2] == ' ' || s[2] == '>')) || (s[1] == 'l' && s[2] == 'i' && (s[3] == ' ' || s[3] == '>'))
            || (s[1] == 'h' && s[2] >= '1' && s[2] <= '6' && (s[3] == ' ' || s[3] == '>'))
            || (s[1] == 'd' && s[2] == 'i' && s[3] == 'v' && (s[4] == ' ' || s[4] == '>')))) {
        plain -= !first && plain[-1] <= ' ';
        *plain = '\n';
        plain += !first;
      }
      while (*s != ' ' && *s != '>' && s < end) s++;
      if (*s == ' ') {
        s++;
        reg = 0;
        while(s < end && *s != '>') {
          reg <<= 8;
          reg |= *s;
          hyperlink = reg == href && *(s + 1) == '=' && *(s + 2) == '\"' && *(s + 3) != '\"';
          if (hyperlink) {
            reg = 0;
            while (s[reg] != '<') reg--;
            nofollow = 0;
            while (s[reg + 14] != '>' && s[reg] != '>') {
              nofollow |= !strncmp(s + reg, "rel=\"nofollow\"", 14);
              reg++;
            }
            s += 3;
            ignore = 1;
            if (!nofollow) {
              reg = 0;
              if (s[0] == '/' && s[1] == '/') {
                s += 2;
                insite = !strncmp(s, host, host_len);
                offset = scheme_len + 3;
                upper_direct = 0;
                ignore = 0;
              } else if (s[0] == '/') {
                insite = 1;
                offset = scheme_len + 3 + host_len;
                upper_direct = 0;
                ignore = 0;
              } else if (s[0] == '.') {
                if (s[1] == '.' && s[2] == '/') {
                  insite = 1;
                  offset = scheme_len + 3 + host_len;
                  s += 3;
                  reg = 1;
                  while (s[0] == '.' && s[1] == '.' && s[2] == '/' && s < end) {
                    reg++;
                    s += 3;
                  }
                  upper_direct = reg;
                  ignore = 0;
                } else if (s[1] == '/'){
                  s += 2;
                  insite = 1;
                  offset = scheme_len + 3 + host_len + file_path_len;
                  upper_direct = 0;
                  ignore = 0;
                }

              } else {
                reg = 0;
                while (s[reg] != '\"' && s[reg] != '/' && s[reg] != ':' && s[reg] != '?' && s[reg] != '#' && s < end) reg++;
                if(s[reg] == ':') {
                  insite = !strncmp(s, scheme_host_file_path, scheme_len + host_len + 3) && (s[scheme_len + host_len + 3] == '\"' || s[scheme_len + host_len + 3] =='/' || s[scheme_len + host_len + 3] == '?' || s[scheme_len + host_len + 3] == '#');
                  offset = 0;
                  upper_direct = 0;
                  ignore = strncmp(s, "https://", 8) && strncmp(s, "http://", 7);
                }
              }
              if (!ignore) {
                if (insite) {
                 if (internal_link) {
                    reg = 0;
                    if (offset) {
                      while (reg < offset && reg < URI_LEN) {
                        uri_buf[reg] = scheme_host_file_path[reg];
                        reg++;
                      }
                    }
                    cnt = 0;
                    if (upper_direct && upper_direct < dirdepth && reg < URI_LEN) {
                      while (cnt < dirdepth - upper_direct && reg < URI_LEN) {
                        uri_buf[reg] = file_path[reg];
                        cnt += file_path[reg] == '/';
                        reg++;
                      }
                    }
                    while (*s != '\"' && *s != '#' && s < end && reg < URI_LEN - 5) {
                      if (*s <= ' ') {
                        s++;
                        continue;
                      } else if (*s == '&') {
                        int n;
                        uint32_t c;
                        c = solve_and_escape(s, &n);
                        s += n;
                        if (!n) {
                          uri_buf[reg++] = *(s++);
                          continue;
                        }
                        if (c <= ' ') continue;
                        if (c > 0xffffff) {
                          uri_buf[reg++] = (c >> 24) & 0xff;
                        }
                        if (reg > 0xffff) {
                          uri_buf[reg++] = (c >> 16) & 0xff;
                        }
                        if (reg > 0xff) {
                          uri_buf[reg++] = (c >> 8) & 0xff;
                        }
                        if (reg > 0) {
                          uri_buf[reg++] = c & 0xff;
                        }
                        continue;
                      }
                      uri_buf[reg++] = *(s++);
                    }
                    if (reg < URI_LEN) {
                      if (reg < scheme_len + 3 + host_len) base[reg++] = '/';
                      uri_buf[reg] = '\0';
                      if (!is_in_aggregate(internal_link, uri_buf)) {
                        add_to_aggregate(internal_link, uri_buf);
                      }
                    }
                  }
                } else {
                 if (outer_link) {
                    reg = 0;
                    if (offset) {
                      while (reg < offset && reg < URI_LEN) {
                        uri_buf[reg] = scheme_host_file_path[reg];
                        reg++;
                      }
                    }
                    cnt = 0;
                    while (*s != '\"' && *s != '#' && s < end && reg < URI_LEN) {
                      if (*s <= ' ') {
                        s++;
                        continue;
                      } else if (*s == '&') {
                        int n;
                        uint32_t c;
                        c = solve_and_escape(s, &n);
                        s += n;
                        if (!n) {
                          uri_buf[reg++] = *(s++);
                          continue;
                        }
                        if (c <= ' ') continue;
                        if (c > 0xffffff) {
                          uri_buf[reg++] = (c >> 24) & 0xff;
                        }
                        if (reg > 0xffff) {
                          uri_buf[reg++] = (c >> 16) & 0xff;
                        }
                        if (reg > 0xff) {
                          uri_buf[reg++] = (c >> 8) & 0xff;
                        }
                        if (reg > 0) {
                          uri_buf[reg++] = c & 0xff;
                        }
                        continue;
                      }
                      uri_buf[reg++] = *(s++);
                    }

                    if (reg < URI_LEN) {
                      uri_buf[reg] = '\0';
                      if (!is_in_aggregate(outer_link, uri_buf)) {
                        add_to_aggregate(outer_link, uri_buf);
                      }
                    }
                  }
                }
              }
              if (*s != '\"') {
                while (*s != '\"' && s < end) s++;
              }
              s++;
              reg = 0;
              continue;
            }
          }
          s++;
        }
      }
      s++;
      if (script) {
        while (strncmp(s, "</script", 8) && strncmp(s, "</SCRIPT", 8) && s < end) {
          if (*s == '\"') {
            while (*s != '\"' && s < end) {
              s += 1 + (*s == '\\');
            }
            s++;
            continue;
          } else if (*s == '\'') {
            while (*s != '\'' && s < end) {
              s += 1 + (*s == '\\');
            }
            s++;
            continue;
          } else if (s[0] == '/' && s[1] == '/') {
            s += 2;
            while (*s != '\n' && s < end) {
              s++;
            }
            s++;
            continue;
          } else if (s[0] == '/' && s[1] == '*') {
            s += 2;
            while (s[0] == '*' && s[1] == '/' && s < end) {
              s++;
            }
            s += 2;
            continue;
          }
          s++;
        }
        while (*s != '>' && s < end) s++;
        s++;
      } else if (style) {
        while (strncmp(s, "</style", 7) && strncmp(s, "</STYLE", 7) && s < end) {
          if (*s == '\"') {
            s++;
            while (*s != '\"') {
              s++;
            }
            s++;
            continue;
          } else if (s[0] == '<' && s[1] == '!' && s[2] == '-') {
            if (s[3] == '-') {
              s += 4;
              while (!(s[0] == '-' && s[1] == '-')) {
                s++;
              }
              s += 2;
              while (*s != '>') s++;
              s++;
            } else {
              s += 3;
              while (*s != '-') s++;
              while (*s != '>') s++;
              s++;
            }
            continue;
          }
          s++;
        }
        while (*s != '>' && s < end) s++;
        s++;
      }
    }
  }
  plain -= !first && plain[-1] <= ' ';
  *(plain++) = '\n';
  *(plain) = '\0';
  free(scheme_host_file_path);
  return 0;
}

#endif
