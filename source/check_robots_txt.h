#ifndef CHECK_ROBOTS_TXT
#define CHECK_ROBOTS_TXT

#include <string.h>
#include <ctype.h>
#include "myfunctions.h"

/*headerで指定したhttpヘッダ文字列を参照し、レスポンスステータスが200番台なら、
  "source"によって指定された"robots.txt"から、すべてのuser-agentに対してアクセスが禁止されているパスを取得します。*/
int check_robots_txt(char* header, char* source, struct str_aggregate* disallowed_linklist) {
  while (*header != ' ' && *header != '\0') header++;
  if (header[1] != '2') return 0;
  register char* s = source;
  register char* end = s + strlen(s);
  char* uri_buf = malloc(URI_LEN);
  while (s < end) {
    if (*s == '#') {
      while (*s != '\n' && s < end) s++;
      s += *s == '\n';
      continue;
    }
    if (!strncmp_alphabet("user-agent:", s, 11)) {
      s += 11;
      while (*s == ' ') s++;
      if (s[0] == '*') {
        while (s < end) {
          if (!strncmp_alphabet("user-agent:", s, 11)) {
            break;
          } else if (!strncmp_alphabet("disallow:", s, 9)) {
            s += 9;
            while (*s == ' ') s++;
            int i = 0;
            while (*s != '#' && *s != '\n' && s < end && i < URI_LEN) {
              uri_buf[i++] = *(s++);
            }
            if (i >= URI_LEN) {
              printf("check_robots_txt: exceeds URI_LEN\n");
              exit(1);
            }
            uri_buf[i] = '\0';
            add_to_aggregate(disallowed_linklist, uri_buf);
            continue;
          } else if (*s == '#') {
            while (*s != '\n' && s < end) s++;
            s += *s == '\n';
            continue;
          }
          s++;
        }
      } else {
        while (s < end) {
          if (!strncmp_alphabet("user-agent:", s, 11)) {
            break;
          } else if (*s == '#') {
            while (*s != '\n' && s < end) s++;
            s += *s == '\n';
            break;
          }
          s++;
        }
      }
      continue;
    }
    s++;
  }
  return 0;
}

#endif
