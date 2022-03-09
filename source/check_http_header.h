#ifndef CHECK_HTTP_HEADER
#define CHECK_HTTP_HEADER
#include <string.h>

//sourceで指定したヘッダーの文字列から、200番代のレスポンスかつ、content-type: text/htmlを含む場合、1を返します。
int check_http_header(char* source) {
  int ishtml = 0;
  register unsigned char* s = source;
  while(*s != ' ' && *s != '\0') s++;
  if (s[1] != '2') {
    printf("\t\t%c%c%c\n\n", s[1], s[2], s[3]);
    return 0;
  }
  for (int i = 0; i < 13; i++) {
    s[i] = (s[i] >= 'A' && s[i] <= 'Z') * (s[i] - 'A' + 'a') + (s[i] < 'A' || s[i] > 'Z') * s[i];
  }
  while (!(s[0] == '\n' && (s[1] == '\n' || s[1] == '<')) && s[0] != '\0') {
    s[13] = (s[13] >= 'A' && s[13] <= 'Z') * (s[13] - 'A' + 'a') + (s[13] < 'A' || s[13] > 'Z') * s[13];
    if (!strncmp(s++, "content-type:", 13)){
      while (s[0] != '\0' && s[8] != '\0' && s[0] != '\n' && s[8] != '\n') {
        s[13] = (s[13] >= 'A' && s[13] <= 'Z') * (s[13] - 'A' + 'a') + (s[13] < 'A' || s[13] > 'Z') * s[13];
        ishtml |= !strncmp(s, "text/html", 9);
        s++;
      }
    }
  }
  if (!ishtml) printf("\t\tnot text/html\n\n");
  return ishtml;
}

#endif
