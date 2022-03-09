#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "myfunctions.h"

//./index/tmpに保存された索引と./indexの索引を並び替えしたうえで統合します

int update_index(unsigned max_mems, int title_weight, int native_weight, int plain_weight) {
  char index_path[256];
  char tmp_path[256];
  strcpy(index_path, "./index/");
  strcpy(tmp_path, "./index/tmp/");
  char* p_i = index_path + strlen(index_path);
  char* p_t = tmp_path + strlen(tmp_path);
  DIR* dir = opendir(tmp_path);
  struct dirent* dp;
  if (dir == NULL) return 0;
  FILE* index;
  FILE* tmp;
  struct str_aggregate titles;
  titles.base = malloc(TITLE_LEN * max_mems);
  titles.current_size = 0;
  titles.max_size = TITLE_LEN * max_mems;
  struct str_aggregate uris;
  uris.base = malloc(URI_LEN * max_mems);
  uris.current_size = 0;
  uris.max_size = URI_LEN * max_mems;
  char** title_p = malloc(sizeof(char*) * max_mems);
  char** uri_p = malloc(sizeof(char*) * max_mems);
  unsigned* num_found_in_title = malloc(sizeof(unsigned*) * max_mems);
  unsigned* num_found_in_native = malloc(sizeof(unsigned*) * max_mems);
  unsigned* num_found_in_plain = malloc(sizeof(unsigned*) * max_mems);
  char title_buf[TITLE_LEN];
  char uri_buf[URI_LEN];
  unsigned nmems;
  while ((dp = readdir(dir)) != NULL) {
    if (!strncmp(dp->d_name + strlen(dp->d_name) - 4, ".txt", 4)) {
      titles.current_size = 0;
      uris.current_size = 0;
      nmems = 0;
      strcpy(p_i, dp->d_name);
      index = fopen(index_path, "r");
      if (index != NULL) {
        while (nmems < max_mems) {
          title_p[nmems] = titles.base + titles.current_size;
          uri_p[nmems] = uris.base + uris.current_size;
          int r = fscanf(index, "%d,%d,%d,", num_found_in_title + nmems,
                  num_found_in_native + nmems, num_found_in_plain + nmems);
          if (r == EOF || r <= 0) break;
          r = fscanf(index, "%[^,],%s\n", title_buf, uri_buf);
          if (r == EOF || r <= 0) break;
          add_to_aggregate(&titles, title_buf);
          add_to_aggregate(&uris, uri_buf);
          nmems++;
        }
        fclose(index);
      }
      strcpy(p_t, dp->d_name);
      tmp = fopen(tmp_path, "r");
      if (tmp != NULL) {
        while (nmems < max_mems) {
          title_p[nmems] = titles.base + titles.current_size;
          uri_p[nmems] = uris.base + uris.current_size;
          int r = fscanf(tmp, "%d,%d,%d,", num_found_in_title + nmems,
                  num_found_in_native + nmems, num_found_in_plain + nmems);
          if (r == EOF || r <= 0) break;
          r = fscanf(tmp, "%[^,],%s\n", title_buf, uri_buf);
          if (r == EOF || r <= 0) break;
          unsigned n = search_in_aggregate(&uris, uri_buf);
          if (n > nmems) {
            add_to_aggregate(&titles, title_buf);
            add_to_aggregate(&uris, uri_buf);
            nmems++;
          } else {
            num_found_in_title[n] = num_found_in_title[nmems];
            num_found_in_native[n] = num_found_in_native[nmems];
            num_found_in_plain[n] = num_found_in_plain[nmems];
          }
        }
        fclose(tmp);
      }
      index = fopen(index_path, "w");
      if (nmems && index) {
        for (unsigned i = 0; i < nmems; i++) {
          unsigned bestmem = i;
          uint64_t bestscore = 0;
          for (unsigned j = i; j < nmems; j++) {
            uint64_t score = num_found_in_title[j] * title_weight
                              + num_found_in_native[j] * native_weight
                                + num_found_in_plain[j] * plain_weight;
            if (score > bestscore) {
              bestmem = j;
              bestscore = score;
            }
          }
          unsigned uswap = num_found_in_title[i];
          num_found_in_title[i] = num_found_in_title[bestmem];
          num_found_in_title[bestmem] = uswap;
          uswap = num_found_in_native[i];
          num_found_in_native[i] = num_found_in_native[bestmem];
          num_found_in_native[bestmem] = uswap;
          uswap = num_found_in_plain[i];
          num_found_in_plain[i] = num_found_in_plain[bestmem];
          num_found_in_plain[bestmem] = uswap;
          char* cswap = title_p[i];
          title_p[i] = title_p[bestmem];
          title_p[bestmem] = cswap;
          cswap = uri_p[i];
          uri_p[i] = uri_p[bestmem];
          uri_p[bestmem] = cswap;
          fprintf(index, "%d,%d,%d,", num_found_in_title[i],
                  num_found_in_native[i], num_found_in_plain[i]);
          fprintf(index, "%s,%s\n", title_p[i], uri_p[i]);
        }
        fclose(index);
      }
      remove(tmp_path);
    }
  }
  closedir(dir);
  return 0;
}
