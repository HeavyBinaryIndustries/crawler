#ifndef ADD_WORD_TO_DICTIONARY
#define ADD_WORD_TO_DICTIONARY

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include "myfunctions.h"

#define MAX_DICTS 64
#define MAX_WORD_LEN 16

//܂wordによって指定したutf8文字列をディレクトリ辞書にget_binary_dictionaryでメモリ上に展開できる形で追加します。
/*マルチバイト文字を一文字としてディレクトリを作成し、それぞれの"index.bin"に次に続く文字、
  そこから分岐する文字の数、その文字が登録単語の最後の文字になるかを書き込みます。*/

int add_word_to_dictionary_utf8(unsigned char* word, unsigned dictnum) {
  if (dictnum >= MAX_DICTS) {
    printf("add_word_to_dictionary: max_dicts = %d\n", MAX_DICTS);
    return -1;
  }
  int word_b_len = strlen(word);
  int word_mb_len = 0;
  int i = 0;
  int j = 0;
  unsigned char converted_word[MAX_WORD_LEN * 4 + 1];
  while (i < word_b_len && word_mb_len < MAX_WORD_LEN) {
    uint32_t k = 0;
    if (word[i] <= ' ') {
      i++;
    } else if (word[i] <= 0x7f) {
      k = convert_ascii(word[i++]);
    } else if (word[i] <= 0xbf) {
      return -1;
    } else if (word[i] <= 0xdf) {
      k = (word[i++] << 8) | word[i++];
    } else if (word[i] <= 0xef) {
      k = convert_utf8_3byte((word[i++] << 16) | (word[i++] << 8) | word[i++]);
    } else if (word[i] <= 0xf7) {
      k = (word[i++] << 24) | (word[i++] << 16) | (word[i++] << 8) | word[i++];
    } else {
      return -1;
    }
    if (k > 0xffffff) {
      converted_word[j++] = k >> 24;
    }
    if (k > 0xffff) {
      converted_word[j++] = (k >> 16) & 0xff;
    }
    if (k > 0xff) {
      converted_word[j++] = (k >> 8) & 0xff;
    }
    if (k) {
      converted_word[j++] = k & 0xff;
    }
    word_mb_len += !!k;
  }
  converted_word[j] = '\0';
  if (word_mb_len >= MAX_WORD_LEN) {
    printf("add_word_to_dictionary_utf8: %s is too long\n", word);
    return -1;
  }
  puts(converted_word);
  unsigned char path[256];
  mkdir("./dictionaries");
  sprintf(path, "./dictionaries/%d", dictnum);
  mkdir(path);
  unsigned char* w = converted_word;
  unsigned char* end = w + strlen(w);
  unsigned char* p = path + strlen(path);
  unsigned b1, b2;
  DIR* dir;
  struct dirent* direntp;
  unsigned char node_1[9];
  unsigned char node_2[9];
  unsigned char* node1 = node_1;
  unsigned char* node2 = node_2;
  FILE* index;
  unsigned char* index_buf = malloc(9 * 0xfffff);
  uint32_t n1, n2, c;
  unsigned char* swap;
  unsigned n_mem;
  int carry;
  n1 = code_to_uint_utf8(w);
  b1 = code_to_hexagonal_string_utf8(w, node1);
  w += b1;
  int already_registerd = 0;
  while (b1) {
    n2 = code_to_uint_utf8(w);
    b2 = code_to_hexagonal_string_utf8(w, node2);
    w += b2;
    strcpy(p, "/index.bin");
    index = fopen(path, "r");
    if (index != NULL) {
      n_mem = fread(index_buf, 9, 0xfffff, index);
      fclose(index);
    } else {
      n_mem = 0;
    }
    already_registerd = 0;
    if (n_mem) {
      int i = 0;
      while (i < n_mem) {
        c = (index_buf[i * 9] << 24) | (index_buf[i * 9 + 1] << 16) | (index_buf[i * 9 + 2] << 8) | index_buf[i * 9 + 3];
        if (c >= n1) break;
        i++;
      }
      if (c == n1) {
        if (n2 != 0) {
          int hit = 0;
          sprintf(p, "/%s", node1);
          dir = opendir(path);
          while (!hit && (direntp = readdir(dir))) {
            hit = !strcmp(node2, direntp->d_name);
          }
          index_buf[i * 9 + 7] += !hit;
          carry = !hit && index_buf[i * 9 + 7] == 0;
          index_buf[i * 9 + 6] += carry;
          carry &= index_buf[i * 9 + 6] == 0;
          index_buf[i * 9 + 5] += carry;
          carry &= index_buf[i * 9 + 5] == 0;
          index_buf[i * 9 + 4] += carry;
        }
        already_registerd = index_buf[i * 9 + 8];
        index_buf[i * 9 + 8] |= n2 == 0;
      } else if (c > n1) {
        for (int j = n_mem; j > i; j--) {
          for (int k = 0; k < 9; k++) {
            index_buf[j * 9 + k] = index_buf[(j - 1) * 9 + k];
          }
        }
        index_buf[i * 9] = n1 >> 24;
        index_buf[i * 9 + 1] = (n1 >> 16) & 0xff;
        index_buf[i * 9 + 2] = (n1 >> 8) & 0xff;
        index_buf[i * 9 + 3] = n1 & 0xff;
        index_buf[i * 9 + 4] = 0;
        index_buf[i * 9 + 5] = 0;
        index_buf[i * 9 + 6] = 0;
        index_buf[i * 9 + 7] = n2 != 0;
        index_buf[i * 9 + 8] = n2 == 0;
        n_mem++;
      } else {
        index_buf[n_mem * 9] = n1 >> 24;
        index_buf[n_mem * 9 + 1] = (n1 >> 16) & 0xff;
        index_buf[n_mem * 9 + 2] = (n1 >> 8) & 0xff;
        index_buf[n_mem * 9 + 3] = n1 & 0xff;
        index_buf[n_mem * 9 + 4] = 0;
        index_buf[n_mem * 9 + 5] = 0;
        index_buf[n_mem * 9 + 6] = 0;
        index_buf[n_mem * 9 + 7] = n2 != 0;
        index_buf[n_mem * 9 + 8] = n2 == 0;
        n_mem++;
      }
    } else {
      index_buf[n_mem * 9] = n1 >> 24;
      index_buf[n_mem * 9 + 1] = (n1 >> 16) & 0xff;
      index_buf[n_mem * 9 + 2] = (n1 >> 8) & 0xff;
      index_buf[n_mem * 9 + 3] = n1 & 0xff;
      index_buf[n_mem * 9 + 4] = 0;
      index_buf[n_mem * 9 + 5] = 0;
      index_buf[n_mem * 9 + 6] = 0;
      index_buf[n_mem * 9 + 7] = n2 != 0;
      index_buf[n_mem * 9 + 8] = n2 == 0;
      n_mem = 1;
    }
    strcpy(p, "/index.bin");
    index = fopen(path, "w");
    if (index != NULL) {
      fwrite(index_buf, 9, n_mem, index);
      fclose(index);
    }
    sprintf(p, "/%s", node1);
    mkdir(path);
    n1 = n2;
    swap = node1;
    node1 = node2;
    node2 = swap;
    p = path + strlen(path);
    b1 = b2;
  }
  free(index_buf);
  return already_registerd;
}

#endif
