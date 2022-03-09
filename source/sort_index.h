#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "myfunctions.h"

//索引を並び返します

int sort_index(char* path, unsigned max_mems,
                int title_weight, int native_weight, int plain_weight) {
  FILE* index = fopen(path, "r");
  if (index == NULL) {
    printf("sort_index: could not open %s\n", path);
    exit(1);
  }
  unsigned* num_found_in_title = malloc(sizeof(unsigned*) * max_mems);
  unsigned* num_found_in_native = malloc(sizeof(unsigned*) * max_mems);
  unsigned* num_found_in_plain = malloc(sizeof(unsigned*) * max_mems);
  char** titles = malloc(sizeof(char*) * max_mems);
  char** uris = malloc(sizeof(char*) * max_mems);
  char* mem_title = malloc(sizeof(char) * max_mems * TITLE_LEN);
  char* mem_uri = malloc(sizeof(char) * max_mems * URI_LEN);
  if (!(num_found_in_title && num_found_in_native && num_found_in_plain && titles && uris && mem_title && mem_uri)) {
      puts("sort_index: could not alloc memory\n");
      exit(1);
  }
  unsigned total_mems = 0;
  while(total_mems < max_mems) {
    if (fscanf(index, "%d,%d,%d,", num_found_in_title + total_mems,
            num_found_in_native + total_mems, num_found_in_plain + total_mems) == EOF) break;
    if (fscanf(index, "%[^,],%s\n", mem_title + total_mems * TITLE_LEN,
            mem_uri + total_mems * URI_LEN) == EOF) break;
    titles[total_mems] = mem_title + total_mems * TITLE_LEN;
    uris[total_mems] = mem_uri + total_mems * URI_LEN;
    total_mems++;
  }
  fclose(index);
  index = fopen(path, "w");
  for (unsigned i = 0; i < total_mems; i++) {
    unsigned best_mem = i;
    uint64_t bestscore = 0;
    for (unsigned j = i; j < total_mems; j++) {
      uint64_t score = num_found_in_title[j] * title_weight
                + num_found_in_native[j] * native_weight
                  + num_found_in_plain[j] * plain_weight;
      if (score > bestscore) {
        bestscore = score;
        best_mem = j;
      }
    }
    unsigned uswap = num_found_in_title[i];
    num_found_in_title[i] = num_found_in_title[best_mem];
    num_found_in_title[best_mem] = uswap;
    uswap = num_found_in_native[i];
    num_found_in_native[i] = num_found_in_native[best_mem];
    num_found_in_native[best_mem] = uswap;
    uswap = num_found_in_plain[i];
    num_found_in_plain[i] = num_found_in_plain[best_mem];
    num_found_in_plain[best_mem] = uswap;
    char* pswap = titles[i];
    titles[i] = titles[best_mem];
    titles[best_mem] = pswap;
    pswap = uris[i];
    uris[i] = uris[best_mem];
    uris[best_mem] = pswap;
    fprintf(index, "%d,%d,%d,", num_found_in_title[i],
            num_found_in_native[i], num_found_in_plain[i]);
    fprintf(index, "%s,%s\n", titles[i], uris[i]);
  }
  fclose(index);
  free(num_found_in_title);
  free(num_found_in_native);
  free(num_found_in_plain);
  free(titles);
  free(uris);
  free(mem_title);
  free(mem_uri);
}
