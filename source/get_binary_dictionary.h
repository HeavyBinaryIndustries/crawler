#ifndef GET_BINARY_DICTIONARY_H
#define GET_BINARY_DICTIONARY_H

#include <stdint.h>
#include <stdio.h>
#include "myfunctions.h"

#define MAX_WORD_LEN 16

//"add_word_to_dictionary_utf8"によって生成されたディレクトリ辞書をメモリ上に展開します
//"mem"にはそれぞれutf8の一文字にあたるバイトを5ビットごとに格納し、それぞれNULL終端させます。
//4バイト文字まで対応しています。
/*"mem_start[0]"は登録単語の一文字目の候補の数を格納しており、"mem[5] ~ mem[9]"が検索するマルチバイト文字の一単位です。
  "mem[5]"~"mem[9]"を一文字にもつ単語の二文字目の候補は"mem[5 * mem_start[1]]"から、"mem[5 * (mem_start[2] - 1)]"までに格納されています。
  is_registerdはその候補が登録単語の最後の文字となるかを示します。
  "mem[5 * mem_start[x]]" ~ "mem[5 * (mem_start[x + 1] - 1)]"の間に次の文字が見つからないか、
  mem_start[x + 1] - mem_start[x] <= 0のとき、次に続く文字はないので、検索は終了となります。*/
  
int get_binary_dictionary(unsigned char* mem, unsigned* mem_start, unsigned char* is_registerd, unsigned dictnum, uint32_t max_mems) {
  char path[256];
  sprintf(path, "./dictionaries/%d", dictnum);
  char* p = path + strlen(path);
  char* base_p = p;
  strcpy(p, "/index.bin");
  FILE* index = fopen(path, "r");
  if (index == NULL) {
    printf("get_binary_dictionary: could not open %s\n", path);
    return 0;
  }
  unsigned char* index_buf = malloc(9 * 0xfffff);
  if (index_buf == NULL) {
    puts("get_binary_dictionary: could not alloc memory");
    return 0;
  }
  unsigned char blanch[5 * MAX_WORD_LEN];
  unsigned n_mem = fread(index_buf, 9, 0xfffff, index);
  fclose(index);
  mem[0] = '\0';
  mem_start[0] = n_mem;
  is_registerd[0] = 0;
  uint32_t total_mem = n_mem + 1;
  mem_start[1] = total_mem;
  FILE* tmp1 = tmpfile();
  FILE* tmp2 = tmpfile();
  FILE* swap;
  unsigned pathes1 = 0;
  unsigned pathes2 = 0;
  for (int i = 0; i < n_mem; i++) {
    int j;
    for (j = 0; j < 4; j++) {
      if (index_buf[i * 9 + j]) break;
    }
    int k;
    for (k = 0; k < 4 - j; k++) {
      mem[(i + 1) * 5 + k] = index_buf[i * 9 + j + k];
    }
    mem[(i + 1) * 5 + k] = '\0';
    total_mem += (index_buf[i * 9 + 4] << 24) | (index_buf[i * 9 + 5] << 16) | (index_buf[i * 9 + 6] << 8) | index_buf[i * 9 + 7];
    mem_start[i + 2] = total_mem;
    is_registerd[i + 1] = index_buf[i * 9 + 8];
    if (total_mem > mem_start[i + 1]) {
      if (total_mem > max_mems) {
        printf("get_binary_dictionary: exceeds max_mems\n");
        return 0;
      }
      p[0] = '/';
      code_to_hexagonal_string_utf8(mem + (i + 1) * 5, p + 1);
      fwrite(path, 1, 256, tmp1);
      fwrite(mem_start + i + 1, sizeof(uint32_t), 1, tmp1);
      pathes1++;
    }
  }
  uint32_t start;
  while (pathes1) {
    rewind(tmp1);
    rewind(tmp2);
    pathes2 = 0;
    for (int i = 0; i < pathes1; i++) {
      fread(path, 1, 256, tmp1);
      fread(&start, sizeof(uint32_t), 1, tmp1);
      p = path + strlen(path);
      sprintf(p, "/index.bin");
      index = fopen(path, "r");
      if (index == NULL) {
        printf("get_binary_dictionary: could not open %s\n", path);
        return 0;
      }
      n_mem = fread(index_buf, 9, 0xfffff, index);
      fclose(index);
      for (int j = 0; j < n_mem; j++) {
        int k;
        for (k = 0; k < 4; k++) {
          if (index_buf[j * 9 + k]) break;
        }
        int l;
        for (l = 0; l < 4 - k; l++) {
          mem[(start + j) * 5 + l] = index_buf[j * 9 + k + l];
        }
        mem[(start + j) * 5 + l] = '\0';
        total_mem += (index_buf[j * 9 + 4] << 24) | (index_buf[j * 9 + 5] << 16) | (index_buf[j * 9 + 6] << 8) | index_buf[j * 9 + 7];
        mem_start[start + j + 1] = total_mem;
        is_registerd[start + j] = index_buf[j * 9 + 8];
        if (total_mem > mem_start[start + j]) {
          if (total_mem > max_mems) {
            printf("get_binary_dictionary: exceeds max_mems\n");
            return 0;
          }
          p[0] = '/';
          code_to_hexagonal_string_utf8(mem + (start + j) * 5, p + 1);
          fwrite(path, 1, 256, tmp2);
          fwrite(mem_start + start + j, sizeof(uint32_t), 1, tmp2);
          pathes2++;
        }
      }
    }
    pathes1 = pathes2;
    swap = tmp1;
    tmp1 = tmp2;
    tmp2 = swap;
  }
  sprintf(path, "./dictionaries/%d/binary_dictionary.bin", dictnum);
  FILE* output = fopen(path, "wb");
  for (int i = 0; i <= total_mem; i++) {
    fwrite(mem + i * 5, 1, 5, output);
    fwrite(mem_start + i, sizeof(uint32_t), 1, output);
    fwrite(is_registerd + i, 1, 1, output);
  }
  fclose(output);
  return total_mem;
}

#endif
