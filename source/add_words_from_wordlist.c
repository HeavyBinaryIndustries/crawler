#include <stdio.h>
#include <string.h>
#include "add_word_to_dictionary.h"

#define MAX_DICTS 64
#define MAX_WORD_LEN 16

//argv[1]によって指定した単語リストに書かれた単語ををargv[2]で指定した番号のディレクトリ辞書に追加します。

int main(int argc, char* argv[]) {
  system("chcp 65001");
  if (argc != 3) {
    printf("usage: %s wordlist dictnum\n", argv[0]);
    return 1;
  }
  unsigned dictnum = (unsigned char)argv[2][0] - '0';
  dictnum *= 10 * (argv[2][1] != '\0');
  dictnum += (argv[2][1] != '\0') * (argv[1][1] - '0');

  if (dictnum >= MAX_DICTS) {
    printf("max_dicts = %d\n", MAX_DICTS);
    return 1;
  }
  FILE* source = fopen(argv[1], "r");
  if (source == NULL) {
    printf("could not open %s\n", argv[1]);
    return 1;
  }
  unsigned char word[256];
  while (fgets(word, 255, source) != NULL) {
    int i;
    for (i = 0; i < 256; i++) {
      if (word[i] <= ' ') break;
    }
    word[i] = '\0';
    if (strlen(word) == 0) continue;
    add_word_to_dictionary_utf8(word, dictnum);
    printf("%s is added\n", word);
  }
  return 0;
}
