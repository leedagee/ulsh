#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

void touch(const char *filename) {
  int fd = open(filename, O_RDONLY | O_CREAT, 0666);
  if (fd == -1) {
    fprintf(stderr, "Cannot open %s: %s\n", filename, strerror(errno));
    return;
  }
  if (futimens(fd, NULL) != 0) {
    fprintf(stderr, "Cannot modify access time of %s: %s\n", filename,
            strerror(errno));
  }
}

int main(int argc, const char *argv[]) {
  for (int i = 1; i < argc; i++) {
    touch(argv[i]);
  }
}
