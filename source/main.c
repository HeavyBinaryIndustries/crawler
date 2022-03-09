#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "extract_plaintext_and_linklist.h"
#include "crawl_in_site.h"
#include "get_binary_dictionary.h"

int main() {
  system("chcp 65001");
  unsigned max_mems = 0xffffff;
  unsigned char* mem = malloc(5 * max_mems);
  uint32_t* mem_start = malloc(sizeof(uint32_t) * max_mems);
  unsigned char* is_registerd = malloc(max_mems);
  get_binary_dictionary(mem, mem_start, is_registerd, 0, max_mems);
  unsigned num_domains = 0;
  char* domain_names = malloc(sizeof(char) * 64 * URI_LEN);
  FILE* fp = fopen("domain_list.txt", "r");
  unsigned j;
  for (int i = 0; i < 64; i++) {
    if (fgets(domain_names + URI_LEN * i, URI_LEN, fp) == NULL) break;
    for (j = 0; j < URI_LEN; j++) {
      if (domain_names[URI_LEN * i + j] == '\n' || domain_names[URI_LEN * i + j] == '\0') break;
    }
    domain_names[URI_LEN * i + j] = '\0';
    num_domains += domain_names[URI_LEN * i] != '\n' && domain_names[URI_LEN * i] != '\0';
  }
  crawl_in_site(domain_names, num_domains, 2, 0xff,
                mem, mem_start, is_registerd, 0xffff);
  return 0;
}
