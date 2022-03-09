#include "sort_index.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <direct.h>

//100:10:1の割合で./index内の索引を並び返します

int main() {
  DIR* dir = opendir("./index");
  struct dirent* direntp;
  if (chdir("./index")) return 0;
  while ((direntp = readdir(dir)) != NULL) {
    if (!strncmp(direntp->d_name + strlen(direntp->d_name) - 4, ".txt", 4)) {
      sort_index(direntp->d_name, 0xffff, 4096, 256, 100, 10, 1);
    }
  }
  closedir(dir);
  chdir("../");
  return 0;
}
