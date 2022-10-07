#include <stdio.h>

int main(int argc, const char *argv[]) {
  for (int i = 1; i < argc; i++) {
    fputs(argv[i], stdout);
    fputc(i + 1 == argc ? '\n' : ' ', stdout);
  }
}
