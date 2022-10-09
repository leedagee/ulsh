#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE (1<<18)

void cat(int fd) {
  char buf[BUFFER_SIZE];
  for (;;) {
    ssize_t len = read(fd, buf, BUFFER_SIZE);
    if (len == 0) {
      // end of file
      return;
    }
    if (len == -1) {
      perror("Cannot read inputs");
      return;
    }
    write(1, buf, len);
  }
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    cat(0);
  }
  for (int i = 1; i < argc; i++) {
    int fd = open(argv[argc], O_RDONLY);
    cat(fd);
    close(fd);
  }
}
