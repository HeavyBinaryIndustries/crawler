#include <stdio.h>
#include <string.h>

//urlをクローリング対象に追加します

int add_url(unsigned char* url) {
  int i = 0;
  int r;
  unsigned char path[256];
  while (url[i] > ' ') i++;
  if (url[i] != '\0') return 1;
  i = 0;
  while (url[i] != ':' && url[i] != '\0') i++;
  if (url[i] = ':') {
    if (strncmp(url, "http://", 7) && strncmp(url, "https://", 8)) return 1;
    i += 3;
    path[0] = '.';
    path[1] = '/';
    int j = 0;
    while (url[i] != '/' && url[i] != '\0') {
      path[j++ + 2] = url[i++];
    }
    if (!j) return 1;
    path[j + 2] = '\0';
    mkdir(path);
    FILE* domain_list = fopen("./domain_list.txt", "a");
    if (domain_list) {
      fputs(path + 2, domain_list);
      fputc('\n', domain_list);
      fclose(domain_list);
    } else return 1;
    strcat(path, "/crawl_option.txt");
    FILE* crawl_option = fopen(path, "a");
    if (crawl_option) {
      fputs(url, crawl_option);
      if (url[i] == '\0') fputc('/', crawl_option);
      fputc('\n', crawl_option);
      fclose(crawl_option);
    } else return 1;
  } else {
    i = 0;
    path[0] = '.';
    path[1] = '/';
    while (url[i] != '/' && url[i] != '\0') {
      path[i + 2] = url[i];
      i++;
    }
    if (!i) return 1;
    path[i + 2] = '\0';
    mkdir(path);
    FILE* domain_list = fopen("./domain_list.txt", "a");
    if (domain_list) {
      fputs(path + 2, domain_list);
      fputc('\n', domain_list);
      fclose(domain_list);
    } else return 1;
    strcat(path, "/crawl_option.txt");
    FILE* crawl_option = fopen(path, "a");
    if (crawl_option) {
      fputs("https://", crawl_option);
      fputs(url, crawl_option);
      if (url[i] == '\0') fputc('/', crawl_option);
      fputc('\n', crawl_option);
      fclose(crawl_option);
    } else return 1;
  }
  return 0;
}
