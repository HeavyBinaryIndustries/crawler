#ifndef INDEXING_H
#define INDEXING_H

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <stdint.h>
#include "myfunctions.h"

#define MAX_WORD_LEN 16

//baseからヌル文字までから"get_binary_dictionary"でメモリ上に展開した辞書を利用して単語を検索します。
//4バイト文字まで対応しています。
/*"mem_start[0]"は登録単語の一文字目の候補の数を格納しており、"mem[5] ~ mem[9]"が検索するマルチバイト文字の一単位です。
  "mem[5]"~"mem[9]"を一文字にもつ単語の二文字目の候補は"mem[5 * mem_start[1]]"から、"mem[5 * (mem_start[2] - 1)]"までに格納されています。
  is_registerdはその候補が登録単語の最後の文字となるかを示します。
  "mem[5 * mem_start[x]]" ~ "mem[5 * (mem_start[x + 1] - 1)]"の間に次の文字が見つからないか、
  mem_start[x + 1] - mem_start[x] <= 0のとき、次に続く文字はないので、検索は終了となります。*/

void find_words_with_binary_dictionary(unsigned char* base, uint32_t* total_words, unsigned char* found_words, uint32_t* num_found,
                                        unsigned char* mem, uint32_t* mem_start, unsigned char* is_registerd, unsigned max_words) {
  unsigned char* s = base;
  unsigned char* end = base + strlen(base);
  unsigned char word[MAX_WORD_LEN * 4 + 1];
  while (base < end) {
    uint32_t search_start = 1;
    uint32_t search_end = mem_start[1];
    unsigned char* w = word;
    s = base;
    while (*s != '\n' && *s != '\0' && s < end && search_end > search_start) {
      int b = pick_char_utf(s, w);
      if (!b) break;
      uint32_t i;
      for (i = search_start; i < search_end; i++) {
        if (!strcmp_reg_inline(mem + i * 5, w)) break;
      }
      if (i >= search_end) break;
      if (is_registerd[i]) {
        uint32_t j;
        for (j = 0; j < *total_words; j++) {
          if (!strcmp_reg_inline(found_words + (MAX_WORD_LEN * 4 + 1) * j, word)) break;
        }
        if (j == *total_words) {
          strcpy(found_words + (MAX_WORD_LEN * 4 + 1) * *total_words, word);
          num_found[*total_words] = 1;
          (*total_words)++;
        } else {
          num_found[j]++;
        }
      }
      search_start = mem_start[i];
      search_end = mem_start[i + 1];
      s += b;
      w += b;
      s += *s == ' ';
    }
    base++;
    base += *base <= ' ';
  }
}

//タイトル、上位か同じディレクトリ内のindex.htmlにない文、平文全体からそれぞれ辞書上の言葉を探し、検索語を16進数に変換したファイル名で索引を作成します。

int indexing_from_plain_native_with_binary_dictionary(char* path, unsigned char* plain, unsigned char* native, unsigned char* title, unsigned char* raw_title, unsigned char* url,
                                                      unsigned char* mem, uint32_t* mem_start, unsigned char* is_registerd, unsigned max_words) {
  unsigned char* found_words = calloc(MAX_WORD_LEN * 4 + 1, max_words);
  uint32_t* num_found_in_title = calloc(sizeof(uint32_t), max_words);
  uint32_t* num_found_in_native = calloc(sizeof(uint32_t), max_words);
  uint32_t* num_found_in_plain = calloc(sizeof(uint32_t), max_words);
  uint32_t total_words = 0;

  find_words_with_binary_dictionary(title, &total_words, found_words, num_found_in_title,
                                    mem, mem_start, is_registerd, max_words);

  find_words_with_binary_dictionary(native, &total_words, found_words, num_found_in_native,
                                    mem, mem_start, is_registerd, max_words);

  find_words_with_binary_dictionary(plain, &total_words, found_words, num_found_in_plain,
                                    mem, mem_start, is_registerd, max_words);

  unsigned char index_path[256];
  sprintf(index_path, "%s/", path);
  unsigned char* p = index_path + strlen(index_path);
  for (int i = 0; i < total_words; i++) {
    utf8_string_to_hexagonal_path(found_words + (MAX_WORD_LEN * 4 + 1) * i, p);
    strcat(index_path, ".txt");
    FILE* index = fopen(index_path, "a+");
    if (index != NULL) {
      fprintf(index, "%d,%d,%d,", num_found_in_title[i], num_found_in_native[i], num_found_in_plain[i]);
      fprintf(index, "%s,", raw_title);
      fprintf(index, "%s\n", url);
      fclose(index);
    }
    printf("\t\t%s, %d, %d, %d\n", found_words + (MAX_WORD_LEN * 4 + 1) * i, num_found_in_title[i], num_found_in_native[i], num_found_in_plain[i]);
  }

  free(found_words);
  free(num_found_in_title);
  free(num_found_in_native);
  free(num_found_in_plain);

  return 0;
}

#endif
