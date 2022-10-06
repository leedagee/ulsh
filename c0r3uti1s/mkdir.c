#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, const char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (mkdir(argv[i], 0777) == -1) /* let umask decide the final mode */ {
      perror("Cannot mkdir");
    }
  }
}