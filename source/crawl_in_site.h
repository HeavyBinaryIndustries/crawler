#ifndef CRAWL_IN_SITE
#define CRAWL_IN_SITE
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include "check_http_header.h"
#include "extract_plaintext_and_linklist.h"
#include "indexing_from_plain_native_with_binary_dictionary.h"
#include "check_robots_txt.h"
#include "update_index.h"
#include "myfunctions.h"

//"host_names"によって指定されたサイトに対し、それぞれのフォルダにある"crawl_option.txt"をもとにクローリングを開始します。
//最大のクローリングuri数は"width",クローリングの深さは"depth"で指定します。
//"mem","mem_start","is_registerd"には"get_binary_dictionary"でメモリ上に展開した辞書のそれぞれのアドレスを充ててください。

/*extract_plaintext_and_linklistによってそれぞれのhtmlの平文を抽出してローカルフォルダに保存し、
  重複がないか確認したうえでサイト内部のリンクを取得してクローリング対象に加えます。*/

/*"indexing_from_plain_native_with_binary_dictionary"によって生成された索引は、"./index"に登録単語のutf8文字列を16進数化したファイル名で保存し、
  関数の終了時に登録単語を発見した数によって、タイトル中の単語は100、平文中の対象サイトの上位ディレクトリや共通ディレクトリの"index.html"にない文字列の単語は10、
  全平文から発見された単語は1の重み付けとして並び替えをします。*/

int crawl_in_site(char* host_names, unsigned num_domains, unsigned depth, unsigned width,
                  unsigned char* mem, uint32_t* mem_start, unsigned char* is_registerd, unsigned max_words) {
  FILE* crawl_option;
  FILE* crawled_uris;
  unsigned char* path = malloc(sizeof(char) * URI_LEN);
  unsigned char* schemes = malloc(sizeof(char) * num_domains * URI_LEN);
  struct str_aggregate* linklists = malloc(sizeof(struct str_aggregate) * num_domains);
  struct str_aggregate* disallowed_linklists = malloc(sizeof(struct str_aggregate) * num_domains);
  char** anchor = malloc(sizeof(char*) * num_domains);
  char** next_uri = malloc(sizeof(char*) * num_domains);
  unsigned* progress = malloc(sizeof(unsigned) * num_domains);
  unsigned* total_links = malloc(sizeof(unsigned) * num_domains);
  if (!(path && schemes && linklists && disallowed_linklists && anchor && next_uri && progress && total_links)) {
    printf("crawl_in_site: failed to alloc memory\n");
    exit(1);
  }

  strcpy(path, "./index/tmp");
  DIR* dir = opendir(path);
  struct dirent* direntp;
  if (dir != NULL) {
    char* p = path + strlen(path);
    while ((direntp = readdir(dir)) != NULL) {
      if (!strncmp(direntp->d_name + strlen(direntp->d_name) - 4, ".txt", 4)) {
        strcpy(p, direntp->d_name);
        remove(path);
      }
    }
  }

  CURL *curl;
  curl = curl_easy_init();
  struct http_buf html_buf;
  html_buf.data_size = 0;
  html_buf.data = NULL;
  struct http_buf header_buf;
  header_buf.data_size = 0;
  header_buf.data = NULL;
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_buf);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buf_write);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_buf);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, buf_write);

  char* uri_buf = malloc(URI_LEN);


  for (int i = 0; i < num_domains; i++) {
    progress[i] = 0;
    total_links[i] = 0;
    linklists[i].base = malloc(0xffffff);
    linklists[i].current_size = 0;
    linklists[i].max_size = 0xffffff;
    disallowed_linklists[i].base = malloc(0xffff);
    disallowed_linklists[i].current_size = 0;
    disallowed_linklists[i].max_size = 0xffff;
    if (!linklists[i].base || !disallowed_linklists[i].base) {
      printf("crawl_in_site: failed to alloc memory\n");
      exit(1);
    }
    sprintf(path, "./%s/crawled_uris.bin", host_names + URI_LEN * i);
    crawled_uris = fopen(path, "r");
    if (crawled_uris != NULL) {
      linklists[i].current_size = fread(linklists[i].base, 1, linklists[i].max_size, crawled_uris);
      fclose(crawled_uris);
    }
    next_uri[i] = linklists[i].base + linklists[i].current_size;
    sprintf(path, "./%s/crawl_option.txt", host_names + URI_LEN * i);
    crawl_option = fopen(path, "r");
    if (crawl_option == NULL) {
      printf("crawl_in_site: could not read %s\n", path);
      exit(1);
    }
    fgets(schemes + URI_LEN * i, 7, crawl_option);
    int j;
    for (j = 0; j < 7; j++) {
      if (schemes[URI_LEN * i + j] == ':') break;
    }
    schemes[URI_LEN * i + j] = '\0';
    if (strcmp("http", schemes + URI_LEN * i) && strcmp("https", schemes + URI_LEN * i)) {
      puts("crawl_in_site: invalid scheme");
      exit(1);
    }
    rewind(crawl_option);
    while (fgets(uri_buf, URI_LEN, crawl_option)) {
      j = 0;
      while (uri_buf[j] != '\n' && uri_buf[j] != '\0') j++;
      uri_buf[j] = '\0';
      add_to_aggregate(linklists + i, uri_buf);
    }
    fclose(crawl_option);
    anchor[i] = linklists[i].base + linklists[i].current_size;
    sprintf(uri_buf, "%s://%s/robots.txt", schemes + URI_LEN * i, host_names + URI_LEN * i);
    header_buf.data_size = 0;
    html_buf.data_size = 0;
    curl_easy_setopt(curl, CURLOPT_URL, uri_buf);
    curl_easy_perform(curl);
    check_robots_txt(header_buf.data, html_buf.data, disallowed_linklists + i);
  }
  free(uri_buf);

  char* plain = malloc(sizeof(char) * BUF_SIZE);
  char* native = malloc(sizeof(char) * BUF_SIZE);
  char* index_buf = malloc(sizeof(char) * BUF_SIZE);
  char* title_buf = malloc(sizeof(char) * TITLE_LEN);
  char* raw_title_buf = malloc(sizeof(char) * TITLE_LEN);
  if (!(plain && native && index_buf && title_buf && raw_title_buf)) {
    printf("crawl_in_site: failed to alloc memory\n");
    exit(1);
  }
  unsigned zero = 0;
  unsigned char* file_path;
  FILE* fplain;
  FILE* fnative;
  FILE* index;
  int ishtml;
  time_t start;
  double wait;
  int end = 0;
  int special;
  unsigned dirback;
  unsigned scheme_len, host_name_len;
  int nread;
  mkdir("./index");
  mkdir("./index/tmp");
  while (!end) {
    end = 1;
    for (int j = 0; j < num_domains; j++) {
      if (total_links[j] > width) continue;
      total_links[j]++;
      if (next_uri[j] >= anchor[j]) {
        anchor[j] = linklists[j].base + linklists[j].current_size;
        progress[j] += progress[j] < depth;
        if (progress[j] >= depth) continue;
        printf("depth: %d %s\n", progress[j], host_names + URI_LEN * j);
      }
      end = 0;
      if (progress[j]) {
        sprintf(path, "./%s/crawled_uris.bin", host_names + URI_LEN * j);
        crawled_uris = fopen(path, "a");
        fputs(next_uri[j], crawled_uris);
        fputc('\0', crawled_uris);
        fclose(crawled_uris);
      }
      if (is_adapt_formats(disallowed_linklists + j, next_uri[j])) {
        printf("\t%s is disallowed by robots.txt", next_uri[j]);
        while (*(next_uri[j])) (next_uri[j])++;
        (next_uri[j])++;
        continue;
      }
      printf("\tGET %s\n", next_uri[j]);
      header_buf.data_size = 0;
      html_buf.data_size = 0;
      curl_easy_setopt(curl, CURLOPT_URL, next_uri[j]);
      curl_easy_perform(curl);
      ishtml = check_http_header(header_buf.data);
      if (ishtml) {
        scheme_len = strlen(schemes + URI_LEN * j);
        host_name_len = strlen(host_names + URI_LEN * j);
        file_path = next_uri[j] + scheme_len + 3 + host_name_len;
        memset(title_buf, 0, TITLE_LEN);
        extract_plaintext_and_linklist(html_buf.data, plain, title_buf, raw_title_buf,
          schemes + URI_LEN * j, host_names + URI_LEN * j, file_path,
          linklists + j, NULL);
        printf("\t\t%s\n\n", raw_title_buf);
        strcpy(native, plain);
        path[0] = '.';
        path[1] = '/';
        strcpy(path + 2, host_names + URI_LEN * j);
        dirback = !chdir(path);
        if (!dirback) {
          printf("could not open %s\n", path);
          exit(1);
        }
        if (next_uri[j] != linklists[j].base) {
          index = fopen("./index_html.txt", "r");
          if (index != NULL) {
            nread = fread(index_buf, 1, BUF_SIZE, index);
            index_buf[nread] = '\0';
            remove_same_string(native, index_buf);
            fclose(index);
          } else {
            dir = opendir("./");
            if (dir != NULL) {
              while (direntp = readdir(dir)) {
                if (!strncmp(direntp->d_name, "index_html-", 11)) {
                  if (direntp->d_name[strlen(direntp->d_name) - 1] == '~') {
                    int n = !chdir(direntp->d_name);
                    if (n) {
                      dir = opendir("./");
                      direntp = readdir(dir);
                      while (direntp->d_name[strlen(direntp->d_name) - 1] == '~') {
                        if (!chdir(direntp->d_name)) {
                          n++;
                          dir = opendir("./");
                          direntp = readdir(dir);
                        } else {
                          break;
                        }
                      }
                      index = fopen(direntp->d_name, "r");
                      if (index != NULL) {
                        nread = fread(index_buf, 1, BUF_SIZE, index);
                        index_buf[nread] = '\0';
                        remove_same_string(native, index_buf);
                        fclose(index);
                      }
                      int k;
                      for (k = 0; k < n; k++) {
                        path[k * 3]     = '.';
                        path[k * 3 + 1] = '.';
                        path[k * 3 + 2] = '/';
                      }
                      path[k * 3] = '\0';
                      chdir(path);
                      path[0] = '.';
                      path[1] = '/';
                    }
                  } else {
                    index = fopen(direntp->d_name, "r");
                    if (index != NULL) {
                      nread = fread(index_buf, 1, BUF_SIZE, index);
                      index_buf[nread] = '\0';
                      remove_same_string(native, index_buf);
                      fclose(index);
                    }
                  }
                  break;
                }
              }
            }
          }
        }
        unsigned char* p = file_path;
        int k = 0;
        while (*p != '\0' && *p != '?') {
          k = 0;
          while (*p != '\0' && *p != '/' && *p != '?' && k < MAX_FNAME - 30) {
            special = *p <= ' ' || *p == '?' || *p == '.' || *p == ',' || *p == '*' || *p == ':' || *p == ';' || *p == '<' || *p == '>' || *p == '|' || *p == '\\' || *p == '&' || *p == '-' || *p == '~';
            path[2 + k++] = (*p * !special) | (special * '_');
            p++;
          }
          if (k >= MAX_FNAME - 30) path[2 + k++] = '-';
          path[2 + k] = '\0';
          if (*p != '\0' && *p != '?' && k) {
            mkdir(path);
            index = fopen("./index_html.txt", "r");
            if (index != NULL) {
              nread = fread(index_buf, 1, BUF_SIZE, index);
              index_buf[nread] = '\0';
              remove_same_string(native, index_buf);
              fclose(index);
            } else {
              dir = opendir("./");
              if (dir != NULL) {
                while (direntp = readdir(dir)) {
                  if (!strncmp(direntp->d_name, "index_html-", 11)) {
                    if (direntp->d_name[strlen(direntp->d_name) - 1] == '-') {
                      int n = !chdir(direntp->d_name);
                      if (n) {
                        dir = opendir("./");
                        direntp = readdir(dir);
                        while (direntp->d_name[strlen(direntp->d_name) - 1] == '-') {
                          if (!chdir(direntp->d_name)) {
                            n++;
                            dir = opendir("./");
                            direntp = readdir(dir);
                          } else {
                            break;
                          }
                        }
                        index = fopen(direntp->d_name, "r");
                        if (index != NULL) {
                          nread = fread(index_buf, 1, BUF_SIZE, index);
                          index_buf[nread] = '\0';
                          remove_same_string(native, index_buf);
                          fclose(index);
                        }
                        int l;
                        for (l = 0; l < n; l++) {
                          path[l * 3]     = '.';
                          path[l * 3 + 1] = '.';
                          path[l * 3 + 2] = '/';
                        }
                        path[l * 3] = '\0';
                        chdir(path);
                        path[0] = '.';
                        path[1] = '/';
                      }
                    } else {
                      index = fopen(direntp->d_name, "r");
                      if (index != NULL) {
                        nread = fread(index_buf, 1, BUF_SIZE, index);
                        index_buf[nread] = '\0';
                        remove_same_string(native, index_buf);
                        fclose(index);
                      }
                    }
                    break;
                  }
                }
              }
            }
            dirback += !chdir(path);
          }
          p += *p == '/';
        }
        if (*p == '?') {
          if (*(p - 1) == '/') {
            sprintf(path + 2, "index_html-");
            k = 11;
          } else {
            path[2 + k++] = '-';
          }
          p++;
          while (*p != '\0') {
            while (*p != '\0' && k < MAX_FNAME - 30) {
              special = *p <= ' ' || *p == '?' || *p == '.' || *p == ',' || *p == '*' || *p == ':' || *p == ';' || *p == '<' || *p == '>' || *p == '|' || *p == '\\' || *p == '&' || *p == '-' || *p == '~';
              path[2 + k++] = (*p * !special) | (special * '_');
              p++;
            }
            if (k >= MAX_FNAME - 30) path[2 + k++] = '-';
            path[2 + k] = '\0';
            if (*p != '\0' && k) {
              mkdir(path);
              dirback += !chdir(path);
              k = 0;
            }
          }
        } else if (*(p - 1) == '/' || p == file_path) {
          sprintf(path + 2, "index_html");
        }
        strcat(path, ".txt");
        if (!(!strcmp(path, "./crawl_option.txt") && !strcmp(path, "./crawled_uris.bin") && dirback == 1)) {
          fplain = fopen(path, "w");
          if (fplain != NULL) {
            fputs(raw_title_buf, fplain);
            fputc('\n', fplain);
            fputs(title_buf, fplain);
            fputc('\n', fplain);
            fputs(plain, fplain);
            fclose(fplain);
          } else {
            printf("could not write %s\n", path);
          }
        }
        strcpy(path + strlen(path) - 4, "_native.txt");
        fnative = fopen(path, "w");
        if (fnative != NULL) {
          fputs(native, fnative);
          fclose(fnative);
        } else {
          printf("could not write %s\n", path);
        }
        int l;
        for (l = 0; l < dirback; l++) {
          path[l * 3]     = '.';
          path[l * 3 + 1] = '.';
          path[l * 3 + 2] = '/';
        }
        path[l * 3] = '\0';
        chdir(path);
        indexing_from_plain_native_with_binary_dictionary("./index/tmp", plain, native, title_buf, raw_title_buf, next_uri[j], mem, mem_start, is_registerd, max_words);
      }
      html_buf.data_size = 0;
      header_buf.data_size = 0;
      while (*(next_uri[j])) (next_uri[j])++;
      (next_uri[j])++;
    }
    wait = 1.0 - difftime(time(NULL), start);
    if (wait > 0.0) {
      printf("waiting for %lf secound\n", wait);
      Sleep(wait * 1000);
    }
  }
  curl_easy_cleanup(curl);
  for (int i = 0; i < num_domains; i++) {
    free(linklists[i].base);
    free(disallowed_linklists[i].base);
  }
  free(linklists);
  free(disallowed_linklists);
  free(path);
  free(schemes);
  free(anchor);
  free(next_uri);
  free(progress);
  free(total_links);
  free(plain);
  free(native);
  free(index_buf);
  free(title_buf);
  free(raw_title_buf);
  free(html_buf.data);
  free(header_buf.data);
  update_index(0xffff, 100, 10, 1);
  return 0;
}

#endif
