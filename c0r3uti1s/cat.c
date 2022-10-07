#include <stdio.h>

#define BUFFER_SIZE 16384

void cat(const char *filename) {
  FILE *fp = fopen(filename, "r");
  char buf[BUFFER_SIZE];
  while (!feof(fp)) {
    size_t len = fread(buf, 1, BUFFER_SIZE, fp);
    fwrite(buf, 1, len, stdout);
  }
  fclose(fp);
}

int main(int argc, const char *argv[]) {
  for (int i = 1; i < argc; i++) {
    cat(argv[i]);
  }
}
