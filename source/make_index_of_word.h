#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include "myfunctions.h"
#include "sort_index.h"

//文字列"cmp"からヌル文字までで文字列"word"を検索し、見つけた数を返します。

int search_word(unsigned char* word, unsigned char* cmp) {
  unsigned char* base = cmp;
  unsigned char* end = cmp + strlen(cmp);
  register unsigned char* w = word;
  int n = 0;
  while (base < end) {
    register unsigned char* c = base;
    w = word;
    while (*w && *c > ' ' && *w == *c) {
      w++;
      c++;
      c += *c == ' ';
    }
    n += !*w;
    base++;
    base += *base <= ' ';
  }
  return n;
}

//文字列"word"を保存したファイルから検索し、索引を作成し、並び替えをします

int make_index_of_word(unsigned char* word, int title_weight, int native_weight, int plain_weight) {
  unsigned char path[256];
  strcpy(path, "./index/");
  utf8_string_to_hexagonal_path(word, path + strlen(path));
  strcat(path, ".txt");
  FILE* index = fopen(path, "w");
  FILE* domain_list = fopen("./domain_list.txt", "r");
  if (!(index && domain_list)) return 0;
  unsigned char* domain_name = malloc(URI_LEN);
  unsigned char* uri_buf = malloc(0xffffff);
  unsigned char* text_buf = malloc(0xffff);
  while (fgets(domain_name, URI_LEN, domain_list) != NULL) {
    if (domain_name[0] < ' ') continue;
    int i = 0;
    while (domain_name[i] > ' ') i++;
    domain_name[i] = '\0';
    puts(domain_name);
    int domain_len = strlen(domain_name);
    sprintf(path, "./%s", domain_name);
    if (chdir(path)) {
      printf("could not open %s\n", domain_name);
      return 1;
    }
    strcpy(path, "./crawled_uris.bin");
    FILE* uris = fopen(path, "r");
    if (uris != NULL) {
      unsigned nread = fread(uri_buf, 1, 0xffffff, uris);
      fclose(uris);
      unsigned char* u = uri_buf;
      while (u < uri_buf + nread) {
        i = 0;
        unsigned char* uri = u;

        while (i < 2) {
          i += *(u++) == '/';
        }
        u += domain_len + 1;
        int dirback = 0;
        FILE* plain = NULL;
        FILE* native = NULL;
        while (u < uri_buf + nread) {
          i = 0;
          path[0] = '.';
          path[1] = '/';

          if (*u == '\0') {
            plain = fopen("./index_html.txt", "r");
            native = fopen("./index_html_native.txt", "r");
            u++;
            break;
          }
          if (*u == '?') {
            sprintf(path, "./index_html-");
            u++;
            i = 11;
          }
          while (*u >= ' ' && *u != '/' && i < MAX_FNAME - 30) {
            int special = *u <= ' ' || *u == '?' || *u == '.' || *u == ',' || *u == '*' || *u == ':' || *u == ';' || *u == '<' || *u == '>' || *u == '|' || *u == '\\' || *u == '&' || *u == '-' || *u == '~';
            path[i++ + 2] =  (*u != '?') * (!special * *u + special * '_')
                              + (*u == '?') * '-';
            u++;
          }
          path[i + 2] = '\0';
          if (*u == '\0') {
            strcpy(path + i + 2, ".txt");
            plain = fopen(path, "r");
            strcpy(path + i + 2, "_native.txt");
            native = fopen(path, "r");
            break;
          } else {
            int r = chdir(path);
            if (r) {
              printf("could not open %s\n", path);
              while (*u && u < uri_buf + nread) u++;
              u++;
              break;
            }
            dirback++;
          }
          u++;
        }
        unsigned char raw_title_buf[TITLE_LEN + 1];
        unsigned char title_buf[TITLE_LEN + 1];
        if (plain != NULL) {
          fgets(raw_title_buf, TITLE_LEN, plain);
          i = 0;
          while (raw_title_buf[i] >= ' ') i++;
          raw_title_buf[i] = '\0';
          fgets(title_buf, TITLE_LEN, plain);
          i = 0;
          while (title_buf[i] >= ' ') i++;
          title_buf[i] = '\0';
          unsigned num_found_in_title = search_word(word, title_buf);
          fread(text_buf, 1, 0xffff, plain);
          fclose(plain);
          unsigned num_found_in_plain = search_word(word, text_buf);
          unsigned num_found_in_native = 0;
          if (native != NULL) {
            fread(text_buf, 1, 0xffff, native);
            fclose(native);
            num_found_in_native = search_word(word, text_buf);
          }
          if (num_found_in_title || num_found_in_native || num_found_in_plain) {
            fprintf(index, "%d,%d,%d,%s,%s\n", num_found_in_title, num_found_in_native, num_found_in_plain, raw_title_buf, uri);
            printf("%d,%d,%d,%s,%s\n", num_found_in_title, num_found_in_native, num_found_in_plain, raw_title_buf, uri);
          }
        }

        if (dirback) {
          i = 0;
          while (i < dirback) {
            path[i * 3] = '.';
            path[i * 3 + 1] = '.';
            path[i * 3 + 2] = '/';
            i++;
          }
          path[i * 3] = '\0';
          chdir(path);
        }
        u += !*u;
      }
    }
    chdir("../");
  }
  fclose(index);
  fclose(domain_list);
  strcpy(path, "./index/");
  utf8_string_to_hexagonal_path(word, path + strlen(path));
  strcat(path, ".txt");
  sort_index(path, 0xffff, title_weight, native_weight, plain_weight);
  free(domain_name);
  free(uri_buf);
  free(text_buf);
  return 0;
}
