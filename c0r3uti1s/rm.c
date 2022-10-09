#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void rm(const char *filename) {
  if (unlink(filename) == -1) {
    fprintf(stderr, "Cannot remove file %s: %s\n", filename, strerror(errno));
  }
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) rm(argv[i]);
}
